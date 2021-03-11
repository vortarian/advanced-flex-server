#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP

#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/functions.h>

using namespace std;

namespace systemicai::http::server::handlers {

// Http Request as a templated alias
template <class Body, class Fields>
using Request = beast::http::request<Body, Fields>;

template <class Body, class Fields>
class Handler {
public:
  Handler(const settings& s) : settings_(s) {
  }

  virtual ~Handler() {
  }

  bool handles(Request<Body, Fields> &req) {
    return true;
  }

  template<class Send>
  void handle(const Request<Body, Fields> &req, Send& send) {
    throw "Not Implemented";
  };

private: 
  Handler(Handler& ) = delete;
  Handler(Handler&&) = delete;
  const Handler& operator=(const Handler&) = delete;

protected:
  const settings &settings_;
};

template< class Body, class Fields>
using HandlerReference = std::shared_ptr< Handler<Body, Fields> >;

// Collection for the handlers
template< class Body, class Fields>
using HandlerCollection = std::list<HandlerReference<Body, Fields> >;

template<class Body, class Fields>
class default_handle_request : public Handler<Body, Fields> {
public:
  default_handle_request(const settings& s) : Handler<Body, Fields>(s) {
  } 

  ~default_handle_request() {
  }

  virtual bool handles(const Request<Body, Fields>& req) {
    // This handles all requests
    return true;
  }
  
  template<class Send>
  void handle(const Request<Body, Fields>& req, Send& send)
  {
    // Returns a bad request response
    auto const bad_request =
            [&req, this](beast::string_view why)
            {
                beast::http::response<beast::http::string_body> res{beast::http::status::bad_request, req.version()};
                res.set(beast::http::field::server, this->settings_.service_version);
                res.set(beast::http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = std::string(why);
                res.prepare_payload();
                return res;
            };

    // Returns a not found response
    auto const not_found =
            [&req, this](beast::string_view target)
            {
                beast::http::response<beast::http::string_body> res{beast::http::status::not_found, req.version()};
                res.set(beast::http::field::server, this->settings_.service_version);
                res.set(beast::http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
                res.body() = "The resource '" + std::string(target) + "' was not found.";
                res.prepare_payload();
                return res;
            };

    // Returns a server error response
    auto const server_error =
            [&req, this](beast::string_view what)
            {
                beast::http::response<beast::http::string_body> res{beast::http::status::internal_server_error, req.version()};
                res.set(beast::http::field::server, this->settings_.service_version);
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
    std::string path = path_cat(this->settings_.document_root, req.target());
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
        res.set(beast::http::field::server, this->settings_.service_version);
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
    res.set(beast::http::field::server, this->settings_.service_version);
    res.set(beast::http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
  }
};


template< class Body, class Fields>
class HandlerRegistry {
public:
  HandlerRegistry(const settings& s) {
    addHandler(std::make_shared< default_handle_request<Body, Fields> >(s));
  }

  HandlerRegistry(const HandlerRegistry& hr) {
    std::copy(hr.begin(), hr.end(), this->handlers_.begin());
  }

  HandlerRegistry(HandlerRegistry&& hr) : handlers_(std::move(hr.handlers_)) {
  }

  void addHandler(HandlerReference<Body, Fields> h) {
      handlers_.insert(handlers_.end(), h);
  }

  const HandlerCollection<Body, Fields> handlers() const {
      return handlers_;
  }
private:
  HandlerCollection<Body, Fields> handlers_;
};

} // namespace systemicai::http::server::handler

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
