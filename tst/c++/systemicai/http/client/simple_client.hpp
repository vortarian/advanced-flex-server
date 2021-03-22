#ifndef SYSTEMICAI_HTTP_CLIENT_SIMPLE_CLIENT_HPP
#define SYSTEMICAI_HTTP_CLIENT_SIMPLE_CLIENT_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = ::boost::beast;     // from <boost/beast.hpp>

// Performs an HTTP GET and returns the response
// Intended for simple testing
template<class Body>
beast::http::response<Body> simple_client_get(const std::string& host, const std::string& port, const std::string& target, int version)
{
    beast::http::response<Body> res;
    try
    {
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);
        auto const results = resolver.resolve(host, port);
        stream.connect(results);
        beast::http::request<beast::http::string_body> req{beast::http::verb::get, target, version};
        req.set(beast::http::field::host, host);
        req.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        beast::http::write(stream, req);
        beast::flat_buffer buffer;
        beast::http::read(stream, buffer, res);
        beast::error_code ec;
        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error from simple_client_get: " << e.what() << std::endl;
        throw;
    }
    return res;
}

#endif // SYSTEMICAI_HTTP_CLIENT_SIMPLE_CLIENT_HPP