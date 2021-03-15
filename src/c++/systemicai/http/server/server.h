#ifndef SYSTEMICAI_HTTP_SERVER_SERVER_H
#define SYSTEMICAI_HTTP_SERVER_SERVER_H

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
#include "sessions.hpp"

namespace systemicai::http::server {

    // Accepts incoming connections and launches the sessions
    class listener : public std::enable_shared_from_this<listener>
    {
    public:
        /**
         * Construct an http listener that allows ssl or plain text sessions.
         * @param ioc IO Context for controlling the http server
         * @param ctx SSL Context for use in ssl settings
         * @param endpoint The tcp endpoint to listen on for connections
         * @param s Configuration settings for the services
         * @param r_ssl A registry of https handlers for ssl which understands the queuing mechanism for the server.
         *  Recommened to use the handler defined by ssl_http_session::type_handler_registry.  Handlers can be added
         *  to the collection to customize behavior.  @see systemicai::http::server::handlers::handler for the base class.
         * @param r_plain A registry of http handlers which understands the queuing mechanism for the server.
         *  Recommened to use the handler defined by plain_http_session::type_handler_registry.  Handlers can be added
         *  to the collection to customize behavior.  @see systemicai::http::server::handlers::handler for the base class.
         */
        listener( net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, const settings& s, const ssl_http_session::type_handler_registry& r_ssl, const plain_http_session::type_handler_registry& r_plain);
        void run();

    private:
        void do_accept();
        void on_accept(beast::error_code ec, tcp::socket socket);

        net::io_context& ioc_;
        ssl::context& ctx_;
        tcp::acceptor acceptor_;
        const settings settings_;
        const ssl_http_session::type_handler_registry& registry_handler_ssl;
        const plain_http_session::type_handler_registry& registry_handler_plain;
    };

} // namespace systemicai::http::server

#endif // SYSTEMICAI_HTTP_SERVER_SERVER_H
