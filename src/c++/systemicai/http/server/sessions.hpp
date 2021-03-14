#ifndef SYSTEMICAI_HTTP_SERVER_SESSIONS_HPP
#define SYSTEMICAI_HTTP_SERVER_SESSIONS_HPP

//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: Advanced server, flex (plain + SSL)
//
//------------------------------------------------------------------------------

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <systemicai/common/certificate.h>
#include <systemicai/http/server/handlers.hpp>
#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/queue.hpp>

#include "boost/beast/http/fields.hpp"
#include "functions.h"
#include "systemicai/http/server/handlers/default.hpp"
#include "systemicai/http/server/handlers/handler.hpp"
#include "systemicai/http/server/handlers/registry.hpp"

namespace systemicai::http::server {

// Echoes back all received WebSocket messages.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template <class Derived> class websocket_session {
  // Access the derived class, this is part of
  // the Curiously Recurring Template Pattern idiom.
  Derived &derived() { return static_cast<Derived &>(*this); }

  beast::flat_buffer buffer_;

  // Start the asynchronous operation
  template <class Body, class Allocator>
  void do_accept(
      beast::http::request<Body, beast::http::basic_fields<Allocator>> req) {
    // Set suggested timeout settings for the websocket
    derived().ws().set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    derived().ws().set_option(
        websocket::stream_base::decorator([](websocket::response_type &res) {
          res.set(beast::http::field::server,
                  std::string(BOOST_BEAST_VERSION_STRING) +
                      " advanced-server-flex");
        }));

    // Accept the websocket handshake
    derived().ws().async_accept(
        req, beast::bind_front_handler(&websocket_session::on_accept,
                                       derived().shared_from_this()));
  }

  void on_accept(beast::error_code ec) {
    if (ec)
      return fail(ec, "accept");

    // Read a message
    do_read();
  }

  void do_read() {
    // Read a message into our buffer
    derived().ws().async_read(
        buffer_, beast::bind_front_handler(&websocket_session::on_read,
                                           derived().shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This indicates that the websocket_session was closed
    if (ec == websocket::error::closed)
      return;

    if (ec)
      return fail(ec, "read");

    // Echo the message
    derived().ws().text(derived().ws().got_text());
    derived().ws().async_write(
        buffer_.data(),
        beast::bind_front_handler(&websocket_session::on_write,
                                  derived().shared_from_this()));
  }

  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Do another read
    do_read();
  }

public:
  // Start the asynchronous operation
  template <class Body, class Allocator>
  void
  run(beast::http::request<Body, beast::http::basic_fields<Allocator>> req) {
    // Accept the WebSocket upgrade request
    do_accept(std::move(req));
  }
};

//------------------------------------------------------------------------------

// Handles a plain WebSocket connection
class plain_websocket_session
    : public websocket_session<plain_websocket_session>,
      public std::enable_shared_from_this<plain_websocket_session> {
  websocket::stream<beast::tcp_stream> ws_;

public:
  // Create the session
  explicit plain_websocket_session(beast::tcp_stream &&stream)
      : ws_(std::move(stream)) {}

  // Called by the base class
  websocket::stream<beast::tcp_stream> &ws() { return ws_; }
};

//------------------------------------------------------------------------------

// Handles an SSL WebSocket connection
class ssl_websocket_session
    : public websocket_session<ssl_websocket_session>,
      public std::enable_shared_from_this<ssl_websocket_session> {
  websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;

public:
  // Create the ssl_websocket_session
  explicit ssl_websocket_session(beast::ssl_stream<beast::tcp_stream> &&stream)
      : ws_(std::move(stream)) {}

  // Called by the base class
  websocket::stream<beast::ssl_stream<beast::tcp_stream>> &ws() { return ws_; }
};

//------------------------------------------------------------------------------

template <class Body, class Allocator>
void make_websocket_session(
    beast::tcp_stream stream,
    beast::http::request<Body, beast::http::basic_fields<Allocator>> req) {
  std::make_shared<plain_websocket_session>(std::move(stream))
      ->run(std::move(req));
}

template <class Body, class Allocator>
void make_websocket_session(
    beast::ssl_stream<beast::tcp_stream> stream,
    beast::http::request<Body, beast::http::basic_fields<Allocator>> req) {
  std::make_shared<ssl_websocket_session>(std::move(stream))
      ->run(std::move(req));
}

//------------------------------------------------------------------------------
typedef beast::http::request_parser<beast::http::string_body> HttpParser;

// Handles an HTTP server connection.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template <class Derived> class http_session {
public:
  typedef handlers::HandlerRegistry< beast::http::fields, queue<http_session> > type_handler_registry;

private:
  // Queue needs access to private data to work effectively
  friend class queue<http_session>;

  // Access the derived class, this is part of
  // the Curiously Recurring Template Pattern idiom.
  Derived &derived() { return static_cast<Derived &>(*this); }

  queue<http_session> queue_;
  const settings &settings_;

  // The parser is stored in an optional container so we can
  // construct it from scratch it at the beginning of each new message.
  boost::optional<HttpParser> parser_;

  // Dynamically provided handlers
  const type_handler_registry &handlers_;

protected:
  beast::flat_buffer buffer_;

public:
  // Construct the session
  http_session(beast::flat_buffer buffer, const settings &s, const type_handler_registry& h)
      : queue_(*this), settings_(s), handlers_(h), buffer_(std::move(buffer)) {
      }

  void do_read() {
    // Construct a new parser for each message
    parser_.emplace();

    // Apply a reasonable limit to the allowed size
    // of the body in bytes to prevent abuse.
    parser_->body_limit(10000);

    // Set the timeout.
    beast::get_lowest_layer(derived().stream())
        .expires_after(std::chrono::seconds(30));

    // Read a request using the parser-oriented interface
    beast::http::async_read(
        derived().stream(), buffer_, *parser_,
        beast::bind_front_handler(&http_session::on_read,
                                  derived().shared_from_this()));
  }

  void on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if (ec == beast::http::error::end_of_stream)
      return derived().do_eof();

    if (ec)
      return fail(ec, "read");

    // See if it is a WebSocket Upgrade
    if (websocket::is_upgrade(parser_->get())) {
      // Disable the timeout.
      // The websocket::stream uses its own timeout settings.
      beast::get_lowest_layer(derived().stream()).expires_never();

      // Create a websocket session, transferring ownership
      // of both the socket and the HTTP request.
      return make_websocket_session(derived().release_stream(),
                                    parser_->release());
    }

    auto req = parser_->release();
    bool handled = false;
    for (auto handler : handlers_.handlers()) 
      if (handler->handles(req)) {
        handled = true;
        handler->handle(req, queue_);
        break;
      }
    if(!handled) {
      const static handlers::internal_server_error< beast::http::fields, queue<http_session> > ise(settings_);
      ise.handle(req, queue_);
    }

    // If we aren't at the queue limit, try to pipeline another request
    if (!queue_.is_full())
      do_read();
  }

  void on_write(bool close, beast::error_code ec,
                std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    if (close) {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      return derived().do_eof();
    }

    // Inform the queue that a write completed
    if (queue_.on_write()) {
      // Read another request
      do_read();
    }
  }
};

//------------------------------------------------------------------------------

// Handles a plain HTTP connection
class plain_http_session
    : public http_session<plain_http_session>,
      public std::enable_shared_from_this<plain_http_session> {
  beast::tcp_stream stream_;

public:
  // Create the session
  plain_http_session(
    beast::tcp_stream &&stream,
    beast::flat_buffer &&buffer,
    const settings &s,
    const http_session<plain_http_session>::type_handler_registry& h
    )
      : http_session<plain_http_session>(std::move(buffer), s, h),
        stream_(std::move(stream)) {}

  // Start the session
  void run() { this->do_read(); }

  // Called by the base class
  beast::tcp_stream &stream() { return stream_; }

  // Called by the base class
  beast::tcp_stream release_stream() { return std::move(stream_); }

  // Called by the base class
  void do_eof() {
    // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
  }
};

//------------------------------------------------------------------------------

// Handles an SSL HTTP connection
class ssl_http_session : public http_session<ssl_http_session>,
                         public std::enable_shared_from_this<ssl_http_session> {
  beast::ssl_stream<beast::tcp_stream> stream_;

public:
  // Create the http_session
  ssl_http_session(
        beast::tcp_stream &&stream
      , ssl::context &ctx
      , beast::flat_buffer &&buffer
      , const settings &s
      , const http_session<ssl_http_session>::type_handler_registry& h
    )
      : http_session<ssl_http_session>(std::move(buffer), s, h),
        stream_(std::move(stream), ctx) {}

  // Start the session
  void run() {
    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    // Note, this is the buffered version of the handshake.
    stream_.async_handshake(
        ssl::stream_base::server, buffer_.data(),
        beast::bind_front_handler(&ssl_http_session::on_handshake,
                                  shared_from_this()));
  }

  // Called by the base class
  beast::ssl_stream<beast::tcp_stream> &stream() { return stream_; }

  // Called by the base class
  beast::ssl_stream<beast::tcp_stream> release_stream() {
    return std::move(stream_);
  }

  // Called by the base class
  void do_eof() {
    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Perform the SSL shutdown
    stream_.async_shutdown(beast::bind_front_handler(
        &ssl_http_session::on_shutdown, shared_from_this()));
  }

private:
  void on_handshake(beast::error_code ec, std::size_t bytes_used) {
    if (ec)
      return fail(ec, "handshake");

    // Consume the portion of the buffer used by the handshake
    buffer_.consume(bytes_used);

    do_read();
  }

  void on_shutdown(beast::error_code ec) {
    if (ec)
      return fail(ec, "shutdown");

    // At this point the connection is closed gracefully
  }
};

} // namespace systemicai::http::server

#endif // SYSTEMICAI_HTTP_SERVER_SESSIONS_HPP