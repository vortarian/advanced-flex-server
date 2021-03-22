#ifndef SYSTEMICAI_HTTP_CLIENT_SSL_CLIENT_HPP
#define SYSTEMICAI_HTTP_CLIENT_SSL_CLIENT_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = ::boost::beast;     // from <boost/beast.hpp>

// Performs an HTTP GET and returns the response
// Intended for simple ssl testing
template<class Body>
beast::http::response<Body> ssl_client_get(boost::asio::ssl::context& ctx, const std::string& host, const std::string& port, const std::string& target, int version)
{
    beast::http::response<Body> res;
    try
    {
        // The io_context is required for all I/O
        boost::asio::io_context ioc;

        // These objects perform our I/O
        boost::asio::ip::tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream).connect(results);

        // Perform the SSL handshake
        stream.handshake(boost::asio::ssl::stream_base::client);
 
        beast::http::request<beast::http::string_body> req{beast::http::verb::get, target, version};
        req.set(beast::http::field::host, host);
        req.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        beast::http::write(stream, req);
        beast::flat_buffer buffer;
        beast::http::read(stream, buffer, res);

        // Gracefully close the stream
        beast::error_code ec;
        stream.shutdown(ec);
        if(ec == boost::asio::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if(ec)
            throw beast::system_error{ec};
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error from ssl_client_get: " << e.what() << std::endl;
        throw;
    }
    return res;
}

#endif // SYSTEMICAI_HTTP_CLIENT_SSL_CLIENT_HPP