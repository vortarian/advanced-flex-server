#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP

#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/functions.h>

using namespace std;

namespace systemicai::http::server::handlers {

template <class Body, class Allocator, class Send> class handler {
public:
  int handle(boost::beast::http::request< Body, boost::beast::http::basic_fields<Allocator> > &req) {
    return 200;
  };
};

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
        Send&& send,
        const settings& s)
{
    // Returns a bad request response
    auto const bad_request =
            [&req, &s](beast::string_view why)
            {
                beast::http::response<beast::http::string_body> res{beast::http::status::bad_request, req.version()};
                res.set(beast::http::field::server, s.service_version);
                res.set(beast::http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = std::string(why);
                res.prepare_payload();
                return res;
            };

    // Returns a not found response
    auto const not_found =
            [&req, &s](beast::string_view target)
            {
                beast::http::response<beast::http::string_body> res{beast::http::status::not_found, req.version()};
                res.set(beast::http::field::server, s.service_version);
                res.set(beast::http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "The resource '" + std::string(target) + "' was not found.";
                res.prepare_payload();
                return res;
            };

    // Returns a server error response
    auto const server_error =
            [&req, &s](beast::string_view what)
            {
                beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, req.version()};
                res.set(beast::http::field::server, s.service_version);
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
        res.set(beast::http::field::server, s.service_version);
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
    res.set(beast::http::field::server, s.service_version);
    res.set(beast::http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
}

} // namespace systemicai::http::server::handler

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
