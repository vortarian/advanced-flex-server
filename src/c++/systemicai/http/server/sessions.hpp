
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

#include <systemicai/http/server/namespace.h>
#include <systemicai/common/certificate.h>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/handlers.hpp>

#include "functions.h"

namespace systemicai::http::server {
    // This function produces an HTTP response for the given
    // request. The type of the response object depends on the
    // contents of the request, so the interface requires the
    // caller to pass a generic lambda for receiving the response.
    template<
            class Body, class Allocator,
            class Send>
    void
    handle_request(
            beast::string_view doc_root,
            beast::http::request<Body, beast::http::basic_fields<Allocator>>&& req,
            Send&& send)
    {
        // Returns a bad request response
        auto const bad_request =
                [&req](beast::string_view why)
                {
                    beast::http::response<beast::http::string_body> res{beast::http::status::bad_request, req.version()};
                    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(beast::http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = std::string(why);
                    res.prepare_payload();
                    return res;
                };

        // Returns a not found response
        auto const not_found =
                [&req](beast::string_view target)
                {
                    beast::http::response<beast::http::string_body> res{beast::http::status::not_found, req.version()};
                    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(beast::http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "The resource '" + std::string(target) + "' was not found.";
                    res.prepare_payload();
                    return res;
                };

        // Returns a server error response
        auto const server_error =
                [&req](beast::string_view what)
                {
                    beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, req.version()};
                    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                    res.set(beast::http::field::content_type, "text/html");
                    res.keep_alive(req.keep_alive());
                    res.body() = "An error occurred: '" + std::string(what) + "'";
                    res.prepare_payload();
                    return res;
                };

        // Make sure we can handle the method
        if( req.method() != beast::http::verb::get &&
            req.method() != beast::http::verb::head)
            return send(bad_request("Unknown HTTP-method"));

        // Request path must be absolute and not contain "..".
        if( req.target().empty() ||
            req.target()[0] != '/' ||
            req.target().find("..") != beast::string_view::npos)
            return send(bad_request("Illegal request-target"));

        // Build the path to the requested file
        std::string path = path_cat(doc_root, req.target());
        if(req.target().back() == '/')
            path.append("index.html");

        // Attempt to open the file
        beast::error_code ec;
        beast::http::file_body::value_type body;
        body.open(path.c_str(), beast::file_mode::scan, ec);

        // Handle the case where the file doesn't exist
        if(ec == beast::errc::no_such_file_or_directory)
            return send(not_found(req.target()));

        // Handle an unknown error
        if(ec)
            return send(server_error(ec.message()));

        // Cache the size since we need it after the move
        auto const size = body.size();

        // Respond to HEAD request
        if(req.method() == beast::http::verb::head)
        {
            beast::http::response<beast::http::empty_body> res{beast::http::status::ok, req.version()};
            res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(beast::http::field::content_type, mime_type(path));
            res.content_length(size);
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }

        // Respond to GET request
        beast::http::response<beast::http::file_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(beast::http::status::ok, req.version())};
        res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(beast::http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    //------------------------------------------------------------------------------

    // Echoes back all received WebSocket messages.
    // This uses the Curiously Recurring Template Pattern so that
    // the same code works with both SSL streams and regular sockets.
    template<class Derived>
    class websocket_session
    {
        // Access the derived class, this is part of
        // the Curiously Recurring Template Pattern idiom.
        Derived&
        derived()
        {
            return static_cast<Derived&>(*this);
        }

        beast::flat_buffer buffer_;

        // Start the asynchronous operation
        template<class Body, class Allocator>
        void
        do_accept(beast::http::request<Body, beast::http::basic_fields<Allocator>> req)
        {
            // Set suggested timeout settings for the websocket
            derived().ws().set_option(
                    websocket::stream_base::timeout::suggested(
                            beast::role_type::server));

            // Set a decorator to change the Server of the handshake
            derived().ws().set_option(
                    websocket::stream_base::decorator(
                            [](websocket::response_type& res)
                            {
                                res.set(beast::http::field::server,
                                        std::string(BOOST_BEAST_VERSION_STRING) +
                                        " advanced-server-flex");
                            }));

            // Accept the websocket handshake
            derived().ws().async_accept(
                    req,
                    beast::bind_front_handler(
                            &websocket_session::on_accept,
                            derived().shared_from_this()));
        }

        void
        on_accept(beast::error_code ec)
        {
            if(ec)
                return fail(ec, "accept");

            // Read a message
            do_read();
        }

        void
        do_read()
        {
            // Read a message into our buffer
            derived().ws().async_read(
                    buffer_,
                    beast::bind_front_handler(
                            &websocket_session::on_read,
                            derived().shared_from_this()));
        }

        void
        on_read(
                beast::error_code ec,
                std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            // This indicates that the websocket_session was closed
            if(ec == websocket::error::closed)
                return;

            if(ec)
                return fail(ec, "read");

            // Echo the message
            derived().ws().text(derived().ws().got_text());
            derived().ws().async_write(
                    buffer_.data(),
                    beast::bind_front_handler(
                            &websocket_session::on_write,
                            derived().shared_from_this()));
        }

        void
        on_write(
                beast::error_code ec,
                std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if(ec)
                return fail(ec, "write");

            // Clear the buffer
            buffer_.consume(buffer_.size());

            // Do another read
            do_read();
        }

    public:
        // Start the asynchronous operation
        template<class Body, class Allocator>
        void
        run(beast::http::request<Body, beast::http::basic_fields<Allocator>> req)
        {
            // Accept the WebSocket upgrade request
            do_accept(std::move(req));
        }
    };

    //------------------------------------------------------------------------------

    // Handles a plain WebSocket connection
    class plain_websocket_session
            : public websocket_session<plain_websocket_session>
                    , public std::enable_shared_from_this<plain_websocket_session>
    {
        websocket::stream<beast::tcp_stream> ws_;

    public:
        // Create the session
        explicit
        plain_websocket_session(
                beast::tcp_stream&& stream)
                : ws_(std::move(stream))
        {
        }

        // Called by the base class
        websocket::stream<beast::tcp_stream>&
        ws()
        {
            return ws_;
        }
    };

    //------------------------------------------------------------------------------

    // Handles an SSL WebSocket connection
    class ssl_websocket_session
            : public websocket_session<ssl_websocket_session>
                    , public std::enable_shared_from_this<ssl_websocket_session>
    {
        websocket::stream<
        beast::ssl_stream<beast::tcp_stream>> ws_;

    public:
        // Create the ssl_websocket_session
        explicit
        ssl_websocket_session(
                beast::ssl_stream<beast::tcp_stream>&& stream)
                : ws_(std::move(stream))
        {
        }

        // Called by the base class
        websocket::stream<
        beast::ssl_stream<beast::tcp_stream>>&
        ws()
        {
            return ws_;
        }
    };

    //------------------------------------------------------------------------------

    template<class Body, class Allocator>
    void
    make_websocket_session(
            beast::tcp_stream stream,
            beast::http::request<Body, beast::http::basic_fields<Allocator>> req)
    {
        std::make_shared<plain_websocket_session>(
                std::move(stream))->run(std::move(req));
    }

    template<class Body, class Allocator>
    void
    make_websocket_session(
            beast::ssl_stream<beast::tcp_stream> stream,
            beast::http::request<Body, beast::http::basic_fields<Allocator>> req)
    {
        std::make_shared<ssl_websocket_session>(
                std::move(stream))->run(std::move(req));
    }

    //------------------------------------------------------------------------------

    // Handles an HTTP server connection.
    // This uses the Curiously Recurring Template Pattern so that
    // the same code works with both SSL streams and regular sockets.
    template<class Derived>
    class http_session
    {
        // Access the derived class, this is part of
        // the Curiously Recurring Template Pattern idiom.
        Derived&
        derived()
        {
            return static_cast<Derived&>(*this);
        }

        // This queue is used for HTTP pipelining.
        class queue
        {
            enum
            {
                // Maximum number of responses we will queue
                limit = 8
            };

            // The type-erased, saved work item
            struct work
            {
                virtual ~work() = default;
                virtual void operator()() = 0;
            };

            http_session& self_;
            std::vector<std::unique_ptr<work>> items_;

        public:
            explicit
            queue(http_session& self)
                    : self_(self)
            {
                static_assert(limit > 0, "queue limit must be positive");
                items_.reserve(limit);
            }

            // Returns `true` if we have reached the queue limit
            bool
            is_full() const
            {
                return items_.size() >= limit;
            }

            // Called when a message finishes sending
            // Returns `true` if the caller should initiate a read
            bool
            on_write()
            {
                BOOST_ASSERT(! items_.empty());
                auto const was_full = is_full();
                items_.erase(items_.begin());
                if(! items_.empty())
                    (*items_.front())();
                return was_full;
            }

            // Called by the HTTP handler to send a response.
            template<bool isRequest, class Body, class Fields>
            void
            operator()(beast::http::message<isRequest, Body, Fields>&& msg)
            {
                // This holds a work item
                struct work_impl : work
                {
                    http_session& self_;
                    beast::http::message<isRequest, Body, Fields> msg_;

                    work_impl(
                            http_session& self,
                            beast::http::message<isRequest, Body, Fields>&& msg)
                            : self_(self)
                            , msg_(std::move(msg))
                    {
                    }

                    void
                    operator()()
                    {
                        beast::http::async_write(
                                self_.derived().stream(),
                                msg_,
                                beast::bind_front_handler(
                                        &http_session::on_write,
                                        self_.derived().shared_from_this(),
                                        msg_.need_eof()));
                    }
                };

                // Allocate and store the work
                items_.push_back(
                        boost::make_unique<work_impl>(self_, std::move(msg)));

                // If there was no previous work, start this one
                if(items_.size() == 1)
                    (*items_.front())();
            }
        };

        std::shared_ptr<std::string const> doc_root_;
        queue queue_;

        // The parser is stored in an optional container so we can
        // construct it from scratch it at the beginning of each new message.
        boost::optional<beast::http::request_parser<beast::http::string_body>> parser_;

    protected:
        beast::flat_buffer buffer_;

    public:
        // Construct the session
        http_session(
                beast::flat_buffer buffer,
                std::shared_ptr<std::string const> const& doc_root)
                : doc_root_(doc_root)
                , queue_(*this)
                , buffer_(std::move(buffer))
        {
        }

        void
        do_read()
        {
            // Construct a new parser for each message
            parser_.emplace();

            // Apply a reasonable limit to the allowed size
            // of the body in bytes to prevent abuse.
            parser_->body_limit(10000);

            // Set the timeout.
            beast::get_lowest_layer(
                    derived().stream()).expires_after(std::chrono::seconds(30));

            // Read a request using the parser-oriented interface
            beast::http::async_read(
                    derived().stream(),
                    buffer_,
                    *parser_,
                    beast::bind_front_handler(
                            &http_session::on_read,
                            derived().shared_from_this()));
        }

        void
        on_read(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            // This means they closed the connection
            if(ec == beast::http::error::end_of_stream)
                return derived().do_eof();

            if(ec)
                return fail(ec, "read");

            // See if it is a WebSocket Upgrade
            if(websocket::is_upgrade(parser_->get()))
            {
                // Disable the timeout.
                // The websocket::stream uses its own timeout settings.
                beast::get_lowest_layer(derived().stream()).expires_never();

                // Create a websocket session, transferring ownership
                // of both the socket and the HTTP request.
                return make_websocket_session(
                        derived().release_stream(),
                        parser_->release());
            }

            // Send the response
            handle_request(*doc_root_, parser_->release(), queue_);

            // If we aren't at the queue limit, try to pipeline another request
            if(! queue_.is_full())
                do_read();
        }

        void
        on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if(ec)
                return fail(ec, "write");

            if(close)
            {
                // This means we should close the connection, usually because
                // the response indicated the "Connection: close" semantic.
                return derived().do_eof();
            }

            // Inform the queue that a write completed
            if(queue_.on_write())
            {
                // Read another request
                do_read();
            }
        }
    };

    //------------------------------------------------------------------------------

    // Handles a plain HTTP connection
    class plain_http_session
            : public http_session<plain_http_session>
                    , public std::enable_shared_from_this<plain_http_session>
    {
        beast::tcp_stream stream_;

    public:
        // Create the session
        plain_http_session(
                beast::tcp_stream&& stream,
                beast::flat_buffer&& buffer,
                std::shared_ptr<std::string const> const& doc_root)
                : http_session<plain_http_session>(
                std::move(buffer),
                doc_root)
                , stream_(std::move(stream))
        {
        }

        // Start the session
        void
        run()
        {
            this->do_read();
        }

        // Called by the base class
        beast::tcp_stream&
        stream()
        {
            return stream_;
        }

        // Called by the base class
        beast::tcp_stream
        release_stream()
        {
            return std::move(stream_);
        }

        // Called by the base class
        void
        do_eof()
        {
            // Send a TCP shutdown
            beast::error_code ec;
            stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

            // At this point the connection is closed gracefully
        }
    };

    //------------------------------------------------------------------------------

    // Handles an SSL HTTP connection
    class ssl_http_session
            : public http_session<ssl_http_session>
                    , public std::enable_shared_from_this<ssl_http_session>
    {
        beast::ssl_stream<beast::tcp_stream> stream_;

    public:
        // Create the http_session
        ssl_http_session(
                beast::tcp_stream&& stream,
                ssl::context& ctx,
                beast::flat_buffer&& buffer,
                std::shared_ptr<std::string const> const& doc_root)
                : http_session<ssl_http_session>(
                std::move(buffer),
                doc_root)
                , stream_(std::move(stream), ctx)
        {
        }

        // Start the session
        void
        run()
        {
            // Set the timeout.
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

            // Perform the SSL handshake
            // Note, this is the buffered version of the handshake.
            stream_.async_handshake(
                    ssl::stream_base::server,
                    buffer_.data(),
                    beast::bind_front_handler(
                            &ssl_http_session::on_handshake,
                            shared_from_this()));
        }

        // Called by the base class
        beast::ssl_stream<beast::tcp_stream>&
        stream()
        {
            return stream_;
        }

        // Called by the base class
        beast::ssl_stream<beast::tcp_stream>
        release_stream()
        {
            return std::move(stream_);
        }

        // Called by the base class
        void
        do_eof()
        {
            // Set the timeout.
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

            // Perform the SSL shutdown
            stream_.async_shutdown(
                    beast::bind_front_handler(
                            &ssl_http_session::on_shutdown,
                            shared_from_this()));
        }

    private:
        void
        on_handshake(
                beast::error_code ec,
                std::size_t bytes_used)
        {
            if(ec)
                return fail(ec, "handshake");

            // Consume the portion of the buffer used by the handshake
            buffer_.consume(bytes_used);

            do_read();
        }

        void
        on_shutdown(beast::error_code ec)
        {
            if(ec)
                return fail(ec, "shutdown");

            // At this point the connection is closed gracefully
        }
    };

} // namespace systemicai::http::server