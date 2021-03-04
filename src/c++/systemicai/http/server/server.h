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

namespace systemicai::http::server {

    // Accepts incoming connections and launches the sessions
    class listener : public std::enable_shared_from_this<listener>
    {
    public:
        listener( net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root, const settings& s);
        void run();

    private:
        void do_accept();
        void on_accept(beast::error_code ec, tcp::socket socket);

        net::io_context& ioc_;
        ssl::context& ctx_;
        tcp::acceptor acceptor_;
        std::shared_ptr<std::string const> doc_root_;
        const settings settings_;
    };

} // namespace systemicai::http::server

#endif // SYSTEMICAI_HTTP_SERVER_SERVER_H
