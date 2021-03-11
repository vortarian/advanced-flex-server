#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP

#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/functions.h>
#include <systemicai/http/server/queue.hpp>

using namespace std;

namespace systemicai::http::server::handlers {

// Http Request as a templated alias
template <class Body, class Fields>
using Request = beast::http::request<Body, Fields>;

template <class Body, class Fields, class Send>
class Handler {
public:
  Handler(const settings& s) : settings_(s) {
  }

  virtual ~Handler() {
  }

  bool handles(Request<Body, Fields> &req) {
    return true;
  }

  virtual void handle(const Request<Body, Fields> &req, Send& send) {
    throw "Not Implemented";
  };

private: 
  Handler(Handler& ) = delete;
  Handler(Handler&&) = delete;
  const Handler& operator=(const Handler&) = delete;

protected:
  const settings &settings_;
};

template< class Body, class Fields, class Send>
using HandlerReference = std::shared_ptr< Handler<Body, Fields, Send> >;

// Collection for the handlers
template< class Body, class Fields, class Send>
using HandlerCollection = std::list<HandlerReference<Body, Fields, Send> >;

} // namespace systemicai::http::server::handler

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
