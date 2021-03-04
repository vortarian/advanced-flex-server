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
#include "server.h"
#include "sessions.hpp"

namespace systemicai::http::server {
    //------------------------------------------------------------------------------

    // Detects SSL handshakes
    class detect_session : public std::enable_shared_from_this<detect_session>
    {
        beast::tcp_stream stream_;
        ssl::context& ctx_;
        std::shared_ptr<std::string const> doc_root_;
        beast::flat_buffer buffer_;
        settings settings_;

    public:
        explicit
        detect_session(
                tcp::socket&& socket,
                ssl::context& ctx,
                std::shared_ptr<std::string const> const& doc_root,
                const settings& s)
                : stream_(std::move(socket))
                , ctx_(ctx)
                , doc_root_(doc_root)
                , settings_(s)
        {
        }

        // Launch the detector
        void
        run()
        {
            // We need to be executing within a strand to perform async operations
            // on the I/O objects in this session. Although not strictly necessary
            // for single-threaded contexts, this example code is written to be
            // thread-safe by default.
            net::dispatch(
                    stream_.get_executor(),
                    beast::bind_front_handler(
                            &detect_session::on_run,
                            this->shared_from_this()));
        }

        void
        on_run()
        {
            // Set the timeout.
            stream_.expires_after(std::chrono::seconds(30));

            beast::async_detect_ssl(
                    stream_,
                    buffer_,
                    beast::bind_front_handler(
                            &detect_session::on_detect,
                            this->shared_from_this()));
        }

        void
        on_detect(beast::error_code ec, bool result)
        {
            if(ec)
                return fail(ec, "detect");

            if(result)
            {
                // Launch SSL session
                std::make_shared<ssl_http_session>(
                        std::move(stream_),
                        ctx_,
                        std::move(buffer_),
                        doc_root_,
                        settings_)->run();
                return;
            }

            // Launch plain session
            std::make_shared<plain_http_session>(
                    std::move(stream_),
                    std::move(buffer_),
                    doc_root_,
                    settings_)->run();
        }
    };

    listener::listener(
            net::io_context& ioc,
            ssl::context& ctx,
            tcp::endpoint endpoint,
            std::shared_ptr<std::string const> const& doc_root,
            const settings& s)
            : std::enable_shared_from_this<listener>()
            , ioc_(ioc)
            , ctx_(ctx)
            , acceptor_(net::make_strand(ioc))
            , doc_root_(doc_root)
            , settings_(s)
            
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
                net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void listener::run()
    {
        do_accept();
    }

    void listener::do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
                net::make_strand(ioc_),
                beast::bind_front_handler(
                        &listener::on_accept,
                        shared_from_this()));
    }

    void listener::on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the detector http_session and run it
            std::make_shared<detect_session>(
                    std::move(socket),
                    ctx_,
                    doc_root_,
                    settings_)->run();
        }

        // Accept another connection
        do_accept();
    }

} // namespace systemicai::http::server