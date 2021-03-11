#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_REGISTRY_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_REGISTRY_HPP

#include "handler.hpp"
#include "default.hpp"

namespace systemicai::http::server::handlers {

template< class Body, class Fields, class Send>
class HandlerRegistry {
public:
  HandlerRegistry(const settings& s) {
    addHandler(std::make_shared< default_handle_request<Body, Fields, Send> >(s));
  }

  HandlerRegistry(const HandlerRegistry& hr) {
    std::copy(hr.begin(), hr.end(), this->handlers_.begin());
  }

  HandlerRegistry(HandlerRegistry&& hr) : handlers_(std::move(hr.handlers_)) {
  }

  void addHandler(HandlerReference<Body, Fields, Send> h) {
      handlers_.insert(handlers_.end(), h);
  }

  const HandlerCollection<Body, Fields, Send> handlers() const {
      return handlers_;
  }
private:
  HandlerCollection<Body, Fields, Send> handlers_;
};

}

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_REGISTRY_HPP