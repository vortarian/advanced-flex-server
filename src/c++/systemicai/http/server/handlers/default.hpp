#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP

#include <systemicai/http/server/namespace.h>
#include "handler.hpp"

namespace systemicai::http::server::handlers {

template< class Body, class Allocator, class Send >
class bad_request : public handler<Body, Allocator, Send> {
public:
  bad_request() : handler<Body, Allocator, Send>() { ; }
  int handle(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator> >& req) {
      return 400;
  };
};

template< class Body, class Allocator, class Send >
class live_request : public handler<Body, Allocator, Send> {
public:
  live_request() : handler<Body, Allocator, Send>() { ; }
  int handle(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator> >& req) {
    return 200;
  };
};

}

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_DEFAULT_HPP