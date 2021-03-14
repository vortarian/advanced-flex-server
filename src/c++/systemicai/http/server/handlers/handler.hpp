#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP

#include "boost/beast/http/detail/type_traits.hpp"
#include <stdexcept>
#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/functions.h>
#include <systemicai/http/server/queue.hpp>

#include <boost/beast/http/message.hpp>

using namespace std;

namespace systemicai::http::server::handlers {

// Base class for the handlers (polymorphism is required for the registry)
template <class Fields, class Send>
class Handler {
public:
  Handler(const settings& s) : settings_(s) {
  }

  virtual ~Handler() {
  }

  virtual bool handles(const beast::http::header<true, Fields> &req) const = 0;
  virtual void handle(const beast::http::header<true, Fields> &req, Send& send) const = 0;

private: 
  Handler(Handler& ) = delete;
  Handler(Handler&&) = delete;
  const Handler& operator=(const Handler&) = delete;

protected:
  const settings &settings_;
};

} // namespace systemicai::http::server::handler

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_HANDLER_HPP
