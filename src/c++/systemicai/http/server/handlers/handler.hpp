#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP

#include <systemicai/http/server/namespace.h>

using namespace std;

namespace systemicai::http::server::handlers {

template <class Body, class Allocator, class Send> class handler {
public:
  int handle(boost::beast::http::request< Body, boost::beast::http::basic_fields<Allocator> > &req) {
    return 200;
  };
};

} // namespace systemicai::http::server::handler

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
