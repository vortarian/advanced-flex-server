//------------------------------------------------------------------------------

#include <thread>

#include <systemicai/http/server/namespace.h>
#include <systemicai/http/server/server.h>
#include <systemicai/common/certificate.h>
#include <systemicai/common/exception.h>


namespace systemicai::http::server {

class service {
  private:
  // The io_context is required for all I/O
  std::shared_ptr<boost::asio::io_context> _ioc;
  ssl::context& _ssl_ctx;
  const settings& settings_;

  public:
  explicit service(const settings& s, ssl::context& sslc) : _ssl_ctx(sslc), settings_(s) {
  }

  /**
   * Start the http service and run until SIGSTOP, SIGTERM or @see this->stop() is called.
   * @return Exit status of the http service (always excess), returns failure if the service is already started
   * @throws Exception on error (unknown type)
   */
  int start()
  {
    if(_ioc) {
      // If this is set, the service is already running
      return EXIT_FAILURE;
    }
    auto const threads = std::max<int>(1, settings_.thread_io);
    _ioc = std::make_shared<boost::asio::io_context>(threads);
    auto const address = boost::asio::ip::make_address(settings_.interface_address.data());
    auto const port = settings_.interface_port;
    auto const doc_root = std::make_shared<string>(settings_.document_root);

    // Create and launch a listening port
    std::make_shared<listener>(
        *_ioc,
        _ssl_ctx,
        boost::asio::ip::tcp::endpoint{address, port},
        doc_root,
        settings_)->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    boost::asio::signal_set signals(*_ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&](beast::error_code const&, int)
        {
          // Stop the `io_context`. This will cause `run()`
          // to return immediately, eventually destroying the
          // `io_context` and all of the sockets in it.
          _ioc->stop();
        });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
      v.emplace_back(
        [this]
        {
          this->_ioc->run();
        }
      );
    _ioc->run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    // Block until all the threads exit
    for(auto& t : v)
      t.join();

    // Reset our io context so we can be started again
    _ioc.reset();

    return EXIT_SUCCESS;
  }



  /**
   * Return true if the service is running and handling requests, false otherwise.
   * @return
   */
  bool running() {
    if(!_ioc)
      return false;
    return !_ioc->stopped();
  }

  /**
   * Calls stop on the io context.  Returns no value, does not block.  Success or failure is returned from the service::start() method
   * @see: boost::asio::io_context::stop
   */
  void stop() {
    _ioc->stop();
  }
};
}
