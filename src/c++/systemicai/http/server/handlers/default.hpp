#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP

#include "boost/beast/http/message.hpp"
#include "boost/beast/http/string_body.hpp"
#include "boost/beast/http/verb.hpp"
#include "handler.hpp"
#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/functions.h>

namespace systemicai::http::server::handlers {

template <class Fields, class Send>
class bad_request : public Handler<Fields, Send> {
public:
  bad_request() : Handler<Fields, Send>() { ; }
  virtual bool handles(const beast::http::request_header<Fields> &req, const settings& s) const {
    return true;
  };

  virtual void handle(const beast::http::request_header<Fields> &req, Send &send, const settings& s) const {
    beast::http::message<true, beast::http::string_body, Fields> req_(req);
    beast::http::response<beast::http::string_body> res{beast::http::status::bad_request, req.version()};
    res.set(beast::http::field::server, s.service_version);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req_.keep_alive());
    res.body() = std::string("Bad Request");
    res.prepare_payload();
    send(std::move(res));
  };
};

template <class Fields, class Send>
class internal_server_error : public Handler<Fields, Send> {
public:
  internal_server_error() : Handler<Fields, Send>() { ; }
  virtual bool handles(const beast::http::request_header<Fields> &req, const settings& s) const {
    return true;
  };

  /**
   * This is mostly thrown when the server is miss-configured, lacks any handlers for a session or all handlers return false on handles()
   */
  virtual void handle(const beast::http::request_header<Fields> &req, Send &send, const settings& s) const {
    beast::http::message<true, beast::http::string_body, Fields> req_(req);
    beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, req.version()};
    res.set(beast::http::field::server, s.service_version);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req_.keep_alive());
    res.body() = std::string("Internal Server Error");
    res.prepare_payload();
    send(std::move(res));
  };
};

template <class Fields, class Send>
class live_request : public Handler<Fields, Send> {
public:
  live_request() : Handler<Fields, Send>() { ; }

  virtual bool handles(const beast::http::request_header<Fields> &req, const settings& s) const {
    if(req.method() == boost::beast::http::verb::get && req.target() == "/live") {
      return true;
    } else {
      return false;
    }
  };

  virtual void handle(const beast::http::request_header<Fields> &req, Send &send, const settings& s) const {
    // TODO: Not all together sure that constructing a new message here is the right approach
    // The queue does use a parser, perhaps we could RTTI to determin if we already have a parsed message?
    beast::http::message<true, beast::http::string_body, Fields> req_(req);
    beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
    res.set(beast::http::field::server, s.service_version);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req_.keep_alive());
    res.body() = "Alive";
    res.prepare_payload();
    send(std::move(res));
  };
};

// TODO: This default handler needs to be refactored for a real world example
// This was carried over from an example of boost::beast
template <class Fields, class Send>
class default_handle_request : public Handler<Fields, Send> {
public:
  default_handle_request() : Handler<Fields, Send>() {}

  ~default_handle_request() {}

  virtual bool handles(const beast::http::request_header<Fields> &req, const settings& s) const {
    // This handles all requests
    return true;
  }

  virtual void handle(const beast::http::request_header<Fields> &req, Send &send, const settings& s) const {
    beast::http::message<true, beast::http::string_body, Fields> req_(req);
    // Returns a bad request response
    auto const bad_request = [&req_, &s](beast::string_view why) {
      beast::http::response<beast::http::string_body> res{
          beast::http::status::bad_request, req_.version()};
      res.set(beast::http::field::server, s.service_version);
      res.set(beast::http::field::content_type, "text/html");
      res.keep_alive(req_.keep_alive());
      res.body() = std::string(why);
      res.prepare_payload();
      return res;
    };

    // Returns a not found response
    auto const not_found = [&req_, &s](beast::string_view target) {
      beast::http::response<beast::http::string_body> res{
          beast::http::status::not_found, req_.version()};
      res.set(beast::http::field::server, s.service_version);
      res.set(beast::http::field::content_type, "text/html");
      res.keep_alive(req_.keep_alive());
      res.body() = "The resource '" + std::string(target) + "' was not found.";
      res.prepare_payload();
      return res;
    };

    // Returns a server error response
    auto const server_error = [&req_, &s](beast::string_view what) {
      beast::http::response<beast::http::string_body> res{
          beast::http::status::internal_server_error, req_.version()};
      res.set(beast::http::field::server, s.service_version);
      res.set(beast::http::field::content_type, "text/html");
      res.keep_alive(req_.keep_alive());
      res.body() = "An error occurred: '" + std::string(what) + "'";
      res.prepare_payload();
      return res;
    };

    // Make sure we can handle the method
    if (req_.method() != beast::http::verb::get &&
        req_.method() != beast::http::verb::head)
      return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req_.target().empty() || req_.target()[0] != '/' ||
        req_.target().find("..") != beast::string_view::npos)
      return send(bad_request("Illegal request-target"));

    // Build the path to the requested file
    std::string path = path_cat(s.document_root, req_.target());
    if (req_.target().back() == '/')
      path.append("index.html");

    // Attempt to open the file
    beast::error_code ec;
    beast::http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == beast::errc::no_such_file_or_directory)
      return send(not_found(req_.target()));

    // Handle an unknown error
    if (ec)
      return send(server_error(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (req_.method() == beast::http::verb::head) {
      beast::http::response<beast::http::empty_body> res{
          beast::http::status::ok, req_.version()};
      res.set(beast::http::field::server, s.service_version);
      res.set(beast::http::field::content_type, mime_type(path));
      res.content_length(size);
      res.keep_alive(req_.keep_alive());
      return send(std::move(res));
    }

    // Respond to GET request
    beast::http::response<beast::http::file_body> res{
        std::piecewise_construct, std::make_tuple(std::move(body)),
        std::make_tuple(beast::http::status::ok, req_.version())};
    res.set(beast::http::field::server, s.service_version);
    res.set(beast::http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req_.keep_alive());
    return send(std::move(res));
  }
};

} // namespace systemicai::http::server::handlers

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP