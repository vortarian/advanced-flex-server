#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP

#include "boost/beast/http/verb.hpp"
#include "handler.hpp"
#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/functions.h>

namespace systemicai::http::server::handlers {

template <class Body, class Fields, class Send>
class bad_request : public Handler<Body, Fields, Send> {
public:
  bad_request() : Handler<Body, Fields, Send>() { ; }
  bool handles(const Request<Body, Fields> &req) {
    return true;
  };

  int handle(const Request<Body, Fields> &req, Send &send) {
    beast::http::response<beast::http::string_body> res{beast::http::status::bad_request, req.version()};
    res.set(beast::http::field::server, this->settings_.service_version);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string("Bad Request");
    res.prepare_payload();
    return Send(res);
  };
};

template <class Body, class Fields, class Send>
class live_request : public Handler<Body, Fields, Send> {
public:
  live_request() : Handler<Body, Fields, Send>() { ; }

  bool handles(const Request<Body, Fields> &req) {
    if(req.method == boost::beast::http::verb::get && req.target == "/live") {
      return true;
    } else {
      return false;
    }
  };

  void handle(const Request<Body, Fields> &req, Send &send) {
    beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
    res.set(beast::http::field::server, this->settings_.service_version);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "Alive";
    res.prepare_payload();
    return Send(res);
  };
};

// TODO: This default handler needs to be refactored for a real world example
// This was carried over from an example of boost::beast
template <class Body, class Fields, class Send>
class default_handle_request : public Handler<Body, Fields, Send> {
public:
  default_handle_request(const settings &s) : Handler<Body, Fields, Send>(s) {}

  ~default_handle_request() {}

  virtual bool handles(const Request<Body, Fields> &req) {
    // This handles all requests
    return true;
  }

  void handle(const Request<Body, Fields> &req, Send &send) {
    // Returns a bad request response
    auto const bad_request = [&req, this](beast::string_view why) {
      beast::http::response<beast::http::string_body> res{
          beast::http::status::bad_request, req.version()};
      res.set(beast::http::field::server, this->settings_.service_version);
      res.set(beast::http::field::content_type, "text/html");
      res.keep_alive(req.keep_alive());
      res.body() = std::string(why);
      res.prepare_payload();
      return res;
    };

    // Returns a not found response
    auto const not_found = [&req, this](beast::string_view target) {
      beast::http::response<beast::http::string_body> res{
          beast::http::status::not_found, req.version()};
      res.set(beast::http::field::server, this->settings_.service_version);
      res.set(beast::http::field::content_type, "text/html");
      res.keep_alive(req.keep_alive());
      res.body() = "The resource '" + std::string(target) + "' was not found.";
      res.prepare_payload();
      return res;
    };

    // Returns a server error response
    auto const server_error = [&req, this](beast::string_view what) {
      beast::http::response<beast::http::string_body> res{
          beast::http::status::internal_server_error, req.version()};
      res.set(beast::http::field::server, this->settings_.service_version);
      res.set(beast::http::field::content_type, "text/html");
      res.keep_alive(req.keep_alive());
      res.body() = "An error occurred: '" + std::string(what) + "'";
      res.prepare_payload();
      return res;
    };

    // Make sure we can handle the method
    if (req.method() != beast::http::verb::get &&
        req.method() != beast::http::verb::head)
      return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req.target().empty() || req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
      return send(bad_request("Illegal request-target"));

    // Build the path to the requested file
    std::string path = path_cat(this->settings_.document_root, req.target());
    if (req.target().back() == '/')
      path.append("index.html");

    // Attempt to open the file
    beast::error_code ec;
    beast::http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == beast::errc::no_such_file_or_directory)
      return send(not_found(req.target()));

    // Handle an unknown error
    if (ec)
      return send(server_error(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (req.method() == beast::http::verb::head) {
      beast::http::response<beast::http::empty_body> res{
          beast::http::status::ok, req.version()};
      res.set(beast::http::field::server, this->settings_.service_version);
      res.set(beast::http::field::content_type, mime_type(path));
      res.content_length(size);
      res.keep_alive(req.keep_alive());
      return send(std::move(res));
    }

    // Respond to GET request
    beast::http::response<beast::http::file_body> res{
        std::piecewise_construct, std::make_tuple(std::move(body)),
        std::make_tuple(beast::http::status::ok, req.version())};
    res.set(beast::http::field::server, this->settings_.service_version);
    res.set(beast::http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
  }
};

} // namespace systemicai::http::server::handlers

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP