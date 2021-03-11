#ifndef SYSTEMICAI_HTTP_SERVER_QUEUE
#define SYSTEMICAI_HTTP_SERVER_QUEUE

#include <vector>
#include <systemicai/http/server/namespace.h>

namespace systemicai::http::server {

template <class Queuable>
// This queue is used for HTTP pipelining.
class queue {
  enum {
    // Maximum number of responses we will queue
    limit = 8
  };

  // The type-erased, saved work item
  struct work {
    virtual ~work() = default;
    virtual void operator()() = 0;
  };

  Queuable &self_;
  std::vector<std::unique_ptr<work>> items_;

public:
  explicit queue(Queuable &self) : self_(self) {
    static_assert(limit > 0, "queue limit must be positive");
    items_.reserve(limit);
  }

  // Returns `true` if we have reached the queue limit
  bool is_full() const { return items_.size() >= limit; }

  // Called when a message finishes sending
  // Returns `true` if the caller should initiate a read
  bool on_write() {
    BOOST_ASSERT(!items_.empty());
    auto const was_full = is_full();
    items_.erase(items_.begin());
    if (!items_.empty())
      (*items_.front())();
    return was_full;
  }

  // Called by the HTTP handler to send a response.
  template <bool isRequest, class Body, class Fields>
  void operator()(beast::http::message<isRequest, Body, Fields> &&msg) {
    // This holds a work item
    struct work_impl : work {
      Queuable &self_;
      beast::http::message<isRequest, Body, Fields> msg_;

      work_impl(Queuable &self,
                beast::http::message<isRequest, Body, Fields> &&msg)
          : self_(self), msg_(std::move(msg)) {}

      void operator()() {
        beast::http::async_write(
            self_.derived().stream(), msg_,
            beast::bind_front_handler(&Queuable::on_write,
                                      self_.derived().shared_from_this(),
                                      msg_.need_eof()));
      }
    };

    // Allocate and store the work
    items_.push_back(boost::make_unique<work_impl>(self_, std::move(msg)));

    // If there was no previous work, start this one
    if (items_.size() == 1)
      (*items_.front())();
  }
};

}

#endif