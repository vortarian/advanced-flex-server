#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP

#include <stdexcept>
#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/functions.h>
#include <systemicai/http/server/queue.hpp>

using namespace std;

namespace systemicai::http::server::handlers {

// Http Request as a templated alias
template <class Body, class Fields>
using Request = beast::http::request<Body, Fields>;

// Base class for the handlers (polymorphism is required for the registry)
template <class Request, class Send>
class Handler {
public:
  Handler(const settings& s) : settings_(s) {
  }

  virtual ~Handler() {
  }

  virtual bool handles(const Request &req) const = 0;
  virtual void handle(const Request &req, Send& send) const = 0;

private: 
  Handler(Handler& ) = delete;
  Handler(Handler&&) = delete;
  const Handler& operator=(const Handler&) = delete;

protected:
  const settings &settings_;
};

} // namespace systemicai::http::server::handler

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
