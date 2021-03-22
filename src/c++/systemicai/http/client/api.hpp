//
// Created by vortarian on 2021/01/19.
// This is an unported client app from a different project, left here as an example as to building a client for this service
//

#ifndef SYSTEMICAI_CLIENT_API_HPP
#define SYSTEMICAI_CLIENT_API_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <systemicai/common/certificate.h>
#include <systemicai/http/client/settings.h>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace beast = boost::beast;
//------------------------------------------------------------------------------

// Report a failure
void fail(boost::system::error_code ec, char const *what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Performs an HTTP GET and prints the response
class signing : public std::enable_shared_from_this<signing> {
  tcp::resolver resolver_;
  ssl::stream<tcp::socket> stream_;
  boost::beast::flat_buffer buffer_; // (Must persist between reads)
  beast::http::request<beast::http::empty_body> req_;
  beast::http::response<beast::http::string_body> res_;

public:
  // Resolver and stream require an io_context
  explicit
  signing(boost::asio::io_context &ioc, ssl::context &ctx)
    : resolver_(ioc), stream_(ioc, ctx) {
  }

  // Start the asynchronous operation
  void run( char const *host, char const *port, char const *target, int version) {
    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
      boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
      std::cerr << ec.message() << "\n";
      return;
    }

    // Set up an HTTP GET request message
    req_.version(version);
    req_.method(beast::http::verb::get);
    req_.target(target);
    req_.set(beast::http::field::host, host);
    req_.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Look up the domain name
    resolver_.async_resolve(
      host,
      port,
      std::bind(
        &signing::on_resolve,
        shared_from_this(),
        std::placeholders::_1,
        std::placeholders::_2));
  }

  void on_resolve( boost::system::error_code ec, tcp::resolver::results_type results) {
    if (ec)
      return fail(ec, "resolve");

    // Make the connection on the IP address we get from a lookup
    boost::asio::async_connect(
      stream_.next_layer(),
      results.begin(),
      results.end(),
      std::bind(
        &signing::on_connect,
        shared_from_this(),
        std::placeholders::_1));
  }

  void on_connect(boost::system::error_code ec) {
    if (ec)
      return fail(ec, "connect");

    // Perform the SSL handshake
    stream_.async_handshake(
      ssl::stream_base::client,
      std::bind(
        &signing::on_handshake,
        shared_from_this(),
        std::placeholders::_1));
  }

  void on_handshake(boost::system::error_code ec) {
    if (ec)
      return fail(ec, "handshake");

    // Send the HTTP request to the remote host
    beast::http::async_write(stream_, req_,
                      std::bind(
                        &signing::on_write,
                        shared_from_this(),
                        std::placeholders::_1,
                        std::placeholders::_2));
  }

  void on_write( boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "write");

    // Receive the HTTP response
    beast::http::async_read(stream_, buffer_, res_,
                     std::bind(
                       &signing::on_read,
                       shared_from_this(),
                       std::placeholders::_1,
                       std::placeholders::_2));
  }

  void on_read( boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec)
      return fail(ec, "read");

    // Write the message to standard out
    std::cout << res_ << std::endl;

    // Gracefully close the stream
    stream_.async_shutdown(
      std::bind(
        &signing::on_shutdown,
        shared_from_this(),
        std::placeholders::_1));
  }

  void on_shutdown(boost::system::error_code ec) {
    if (ec == boost::asio::error::eof) {
      // Rationale:
      // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
      ec.assign(0, ec.category());
    }
    if (ec)
      return fail(ec, "shutdown");

    // If we get here then the connection is closed gracefully
  }
};


// Performs an HTTP GET and prints the response
class master : public std::enable_shared_from_this<master> {
    tcp::resolver resolver_;
    ssl::stream<tcp::socket> stream_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    beast::http::request<beast::http::empty_body> req_;
    beast::http::response<beast::http::string_body> res_;

public:
    // Resolver and stream require an io_context
    explicit
    master(boost::asio::io_context &ioc, ssl::context &ctx)
    : resolver_(ioc), stream_(ioc, ctx) {
    }

    // Start the asynchronous operation
    void run( char const *host, char const *port, char const *target, int version) {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }

        // Set up an HTTP GET request message
        req_.version(version);
        req_.method(beast::http::verb::get);
        req_.target(target);
        req_.set(beast::http::field::host, host);
        req_.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Look up the domain name
        resolver_.async_resolve(
                host,
                port,
                std::bind(
                        &master::on_resolve,
                        shared_from_this(),
                        std::placeholders::_1,
                        std::placeholders::_2));
    }

    void on_resolve( boost::system::error_code ec, tcp::resolver::results_type results) {
        if (ec)
            return fail(ec, "resolve");

        // Make the connection on the IP address we get from a lookup
        boost::asio::async_connect(
                stream_.next_layer(),
                results.begin(),
                results.end(),
                std::bind(
                        &master::on_connect,
                        shared_from_this(),
                        std::placeholders::_1));
    }

    void on_connect(boost::system::error_code ec) {
        if (ec)
            return fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(
                ssl::stream_base::client,
                std::bind(
                        &master::on_handshake,
                        shared_from_this(),
                        std::placeholders::_1));
    }

    void on_handshake(boost::system::error_code ec) {
        if (ec)
            return fail(ec, "handshake");

        // Send the HTTP request to the remote host
        beast::http::async_write(stream_, req_,
                          std::bind(
                                  &master::on_write,
                                  shared_from_this(),
                                  std::placeholders::_1,
                                  std::placeholders::_2));
    }

    void on_write( boost::system::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        // Receive the HTTP response
        beast::http::async_read(stream_, buffer_, res_,
                         std::bind(
                                 &master::on_read,
                                 shared_from_this(),
                                 std::placeholders::_1,
                                 std::placeholders::_2));
    }

    void on_read( boost::system::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "read");

        // Write the message to standard out
        std::cout << res_ << std::endl;

        // Gracefully close the stream
        stream_.async_shutdown(
                std::bind(
                        &master::on_shutdown,
                        shared_from_this(),
                        std::placeholders::_1));
    }

    void on_shutdown(boost::system::error_code ec) {
        if (ec == boost::asio::error::eof) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }
        if (ec)
            return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }
};

// Performs an HTTP GET and prints the response
class child : public std::enable_shared_from_this<child> {
    tcp::resolver resolver_;
    ssl::stream<tcp::socket> stream_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    beast::http::request<beast::http::empty_body> req_;
    beast::http::response<beast::http::string_body> res_;

public:
    // Resolver and stream require an io_context
    explicit
    child(boost::asio::io_context &ioc, ssl::context &ctx)
    : resolver_(ioc), stream_(ioc, ctx) {
    }

    // Start the asynchronous operation
    void run( char const *host, char const *port, char const *target, int version) {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }

        // Set up an HTTP GET request message
        req_.version(version);
        req_.method(beast::http::verb::get);
        req_.target(target);
        req_.set(beast::http::field::host, host);
        req_.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Look up the domain name
        resolver_.async_resolve(
                host,
                port,
                std::bind(
                        &child::on_resolve,
                        shared_from_this(),
                        std::placeholders::_1,
                        std::placeholders::_2));
    }

    void on_resolve( boost::system::error_code ec, tcp::resolver::results_type results) {
        if (ec)
            return fail(ec, "resolve");

        // Make the connection on the IP address we get from a lookup
        boost::asio::async_connect(
                stream_.next_layer(),
                results.begin(),
                results.end(),
                std::bind(
                        &child::on_connect,
                        shared_from_this(),
                        std::placeholders::_1));
    }

    void on_connect(boost::system::error_code ec) {
        if (ec)
            return fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(
                ssl::stream_base::client,
                std::bind(
                        &child::on_handshake,
                        shared_from_this(),
                        std::placeholders::_1));
    }

    void on_handshake(boost::system::error_code ec) {
        if (ec)
            return fail(ec, "handshake");

        // Send the HTTP request to the remote host
        beast::http::async_write(stream_, req_,
                          std::bind(
                                  &child::on_write,
                                  shared_from_this(),
                                  std::placeholders::_1,
                                  std::placeholders::_2));
    }

    void on_write( boost::system::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        // Receive the HTTP response
        beast::http::async_read(stream_, buffer_, res_,
                         std::bind(
                                 &child::on_read,
                                 shared_from_this(),
                                 std::placeholders::_1,
                                 std::placeholders::_2));
    }

    void on_read( boost::system::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "read");

        // Write the message to standard out
        std::cout << res_ << std::endl;

        // Gracefully close the stream
        stream_.async_shutdown(
                std::bind(
                        &child::on_shutdown,
                        shared_from_this(),
                        std::placeholders::_1));
    }

    void on_shutdown(boost::system::error_code ec) {
        if (ec == boost::asio::error::eof) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }
        if (ec)
            return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }
};

#endif // SYSTEMICAI_CLIENT_API_HPP
