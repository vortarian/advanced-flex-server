#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP

#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/functions.h>

using namespace std;

namespace systemicai::http::server::handlers {

template <class Body, class Allocator, class Send> class handler {
public:
  int handle(boost::beast::http::request< Body, boost::beast::http::basic_fields<Allocator> > &req, Send& send) {
    return 200;
  };
};

// Http Request as a templated alias
template <class Body, class Allocator>
using Request = beast::http::request<Body, beast::http::basic_fields<Allocator>>;

// Handler Function as a templated alias
// TODO: Not using move sematics here because the functions will execute in a loop
// Determine if we should switch this to move semantics and add in a handles(...) type of method
template< class Body, class Allocator, class Send> 
using HandlerFunction = bool(*)(Request<Body, Allocator>& req, Send& send, const settings&);

// Collection for the handlers
template< class Body, class Allocator, class Send>
using HandlerCollection = std::list<HandlerFunction<Body, Allocator, Send> >;

// Setup a registry for _compile time_ addition of handlers for the service
template< class Body, class Allocator, class Send>
class HandlerRegistry {
public:
  HandlerRegistry() {
  }

  HandlerRegistry(const HandlerRegistry& hr) {
    std::copy(hr.begin(), hr.end(), this->handlers_.begin());
  }

  HandlerRegistry(HandlerRegistry&& hr) : handlers_(std::move(hr.handlers_)) {
  }

  void addHandler(HandlerFunction<Body, Allocator, Send>* h) {
      handlers_.insert(handlers_.end(), h);
  }

  const HandlerCollection<Body, Allocator, Send> handlers() const {
      return handlers_;
  }
private:
  HandlerCollection<Body, Allocator, Send> handlers_;
};

template< class Body, class Allocator, class Send>
class GlobalHandlerRegistry : public HandlerRegistry< Body, Allocator, Send>
{
public:
  /**
   * Provide access to a global registry
   */
  static GlobalHandlerRegistry& global() {
     static GlobalHandlerRegistry<Body, Allocator, Send> registry;
     return registry;
  }

  /**
   * Provide a means of getting a copy of the handlers since we are blocking all use of constructors & operators
   */
  HandlerRegistry<Body, Allocator, Send>&& toHandlerRegistry() {
    HandlerRegistry<Body, Allocator, Send> hr;
    std::copy(this->begin(), this->end(), hr.handlers_.begin());
    return hr;
  }

private:
  GlobalHandlerRegistry() { ; }

  // Prevent Move Semantics and construction
  GlobalHandlerRegistry(GlobalHandlerRegistry&&) = delete;
  GlobalHandlerRegistry(GlobalHandlerRegistry&) = delete;
  GlobalHandlerRegistry& operator=(const GlobalHandlerRegistry<Body, Allocator, Send>&) = delete;
};

static HandlerRegistry< class Body, class Allocator, class Send> Registry;

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
        class Body, class Allocator,
        class Send>
void
default_handle_request(
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

// This handler allows override of the default handlers by overriding the assigned handler.
// If no handlers are registered with the GlobalHandlerRegistry then it will call default_handle_request
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
  auto gHandlers = GlobalHandlerRegistry<Body, Allocator, Send>::global().handlers();
  if(gHandlers.empty()) {
    bool handled = false;
    for(auto handler : gHandlers) {
        handled = handler(req, send, s);
        if(handled) break;
    }
    if(!handled) {
      default_handle_request(s.document_root, std::move(req), std::move(send), s);
    }
  } else {
      default_handle_request(s.document_root, std::move(req), std::move(send), s);
  }
}

} // namespace systemicai::http::server::handler

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
