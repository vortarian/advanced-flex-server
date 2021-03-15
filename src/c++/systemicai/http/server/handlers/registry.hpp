#ifndef SYSTEMICAI_HTTP_SERVER_HANDLERS_REGISTRY_HPP
#define SYSTEMICAI_HTTP_SERVER_HANDLERS_REGISTRY_HPP

#include "handler.hpp"
#include "default.hpp"
#include <iostream>

namespace systemicai::http::server::handlers {

template< class Fields, class Send >
class HandlerRegistry {
public:
  typedef Fields type_fields;
  typedef Send type_send;
  typedef std::shared_ptr< Handler<type_fields, type_send> > HandlerReference;
  typedef std::list<HandlerReference> HandlerCollection;

  HandlerRegistry() {
    // Setup these handlers by default
    addHandler(std::make_shared< default_handle_request<type_fields, type_send> >());
    addHandler(std::make_shared< live_request<type_fields, type_send> >());
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