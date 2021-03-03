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

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "systemicai/common/certificate.h"
#include "systemicai/http/server/settings.h"
#include "systemicai/http/server/handler.hpp"

#include "functions.h"

namespace systemicai::http::server {

    namespace beast = boost::beast;                 // from <boost/beast.hpp>
    namespace net = boost::asio;                    // from <boost/asio.hpp>
    namespace ssl = boost::asio::ssl;               // from <boost/asio/ssl.hpp>
    using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

    // Accepts incoming connections and launches the sessions
    class listener : public std::enable_shared_from_this<listener>
    {
    public:
        listener( net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root);
        void run();

    private:
        void do_accept();
        void on_accept(beast::error_code ec, tcp::socket socket);

        net::io_context& ioc_;
        ssl::context& ctx_;
        tcp::acceptor acceptor_;
        std::shared_ptr<std::string const> doc_root_;
    };

} // namespace systemicai::http::server

#endif // SYSTEMICAI_HTTP_SERVER_SERVER_H
