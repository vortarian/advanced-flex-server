#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_REGISTRY_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_REGISTRY_HPP

#include "handler.hpp"
#include "default.hpp"
#include <iostream>

namespace systemicai::http::server::handlers {

template< class Fields, class Send >
class HandlerRegistry {
public:
  typedef std::shared_ptr< Handler<Fields, Send> > HandlerReference;
  typedef std::list<HandlerReference> HandlerCollection;

  HandlerRegistry(const settings& s) {
    // Setup these handlers by default
    addHandler(std::make_shared< default_handle_request<Fields, Send> >(s));
    addHandler(std::make_shared< live_request<Fields, Send> >(s));
  }

  HandlerRegistry(const HandlerRegistry& hr) {
    std::copy(hr.handlers_.begin(), hr.handlers_.end(), this->handlers_.begin());
  }

  HandlerRegistry(HandlerRegistry&& hr) : handlers_(std::move(hr.handlers_)) {
  }

  /**
    Pushes a handler onto the list of handlers in the registry
   */
  void addHandler(HandlerReference h) {
      handlers_.push_front(h);
  }

  const HandlerCollection& handlers() const {
      return handlers_;
  }
private:
  HandlerCollection handlers_;
};

}

#endif // SYSTEMICAI_HTTP_SERVER_HANDLERS_REGISTRY_HPP