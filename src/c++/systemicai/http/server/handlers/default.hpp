#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP

#include <systemicai/http/server/namespace.h>
#include "handler.hpp"

namespace systemicai::http::server::handlers {

template< class Body, class Fields >
class bad_request : public Handler<Body, Fields> {
public:
  bad_request() : Handler<Body, Fields>() { ; }
  bool handles(const Request<Body, Fields>& req) {
      return true;
  };
  template<class Send>
  int handle(const Request<Body, Fields>& req, Send& send) {
      beast::http::response<beast::http::string_body> res{beast::http::status::bad_request, req.version()};
      res.set(beast::http::field::server, this->settings_.service_version);
      res.set(beast::http::field::content_type, "text/html");
      res.keep_alive(req.keep_alive());
      res.body() = std::string("Bad Request");
      res.prepare_payload();
      return Send(res);
  };
};

template< class Body, class Fields >
class live_request : public Handler<Body, Fields> {
public:
  live_request() : Handler<Body, Fields>() { ; }
  bool handles(const Request<Body, Fields>& req) {
    // TODO: If GET/HEAD and /live
      return true;
  };
  template<class Send>
  void handle(const Request<Body, Fields>& req, Send& send) {
    beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
    res.set(beast::http::field::server, this->settings_.service_version);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "Alive";
    res.prepare_payload();
    return Send(res);
  };
};

}

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP