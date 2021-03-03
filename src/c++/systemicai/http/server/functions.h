#ifndef SYSTEMICAI_HTTP_SERVER_FUNCTIONS_H
#define SYSTEMICAI_HTTP_SERVER_FUNCTIONS_H

//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#include <boost/beast/core.hpp>

namespace systemicai::http::server {

    namespace beast = boost::beast; // from <boost/beast.hpp>

    // Return a reasonable mime type based on the extension of a file.
    beast::string_view mime_type(beast::string_view path);

    // Append an HTTP rel-path to a local filesystem path.
    // The returned path is normalized for the platform.
    std::string path_cat( beast::string_view base, beast::string_view path);

    // Report a failure
    void fail(beast::error_code ec, char const* what);

} // namespace systemicai::http::server  

#endif // SYSTEMICAI_HTTP_SERVER_FUNCTIONS_H