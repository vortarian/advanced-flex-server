//------------------------------------------------------------------------------

#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <boost/property_tree/json_parser.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <systemicai/http/server/server.hpp>

using namespace systemicai::http::server;
using namespace std;

namespace systemicai::http::server {

class service {
  private:
  // The io_context is required for all I/O
  std::shared_ptr<boost::asio::io_context> _ioc;
  const settings &_s;

  //------------------------------------------------------------------------------
  void load_server_certificate(boost::asio::ssl::context& ctx)
  {
    ifstream cs(_s.ssl_certificate), ks(_s.ssl_key), ds(_s.ssl_dh);

    std::string const cert(istreambuf_iterator<char>{cs}, {});
    std::string const key(istreambuf_iterator<char>{ks}, {});
    std::string const dh(istreambuf_iterator<char>{ds}, {});

    ctx.set_password_callback(
        [](std::size_t,
           boost::asio::ssl::context_base::password_purpose)
        {
          return "test";
        });

    ctx.set_options(
        boost::asio::ssl::context::default_workarounds |
        boost::asio::ssl::context::no_sslv2 |
        boost::asio::ssl::context::single_dh_use);

    ctx.use_certificate_chain(
        boost::asio::buffer(cert.data(), cert.size()));

    ctx.use_private_key(
        boost::asio::buffer(key.data(), key.size()),
        boost::asio::ssl::context::file_format::pem);

    ctx.use_tmp_dh(
        boost::asio::buffer(dh.data(), dh.size()));
  }

  public:
  explicit service(const settings& s) : _s(s) {
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
    auto const threads = std::max<int>(1, _s.thread_io);
    _ioc = std::make_shared<boost::asio::io_context>(threads);
    auto const address = boost::asio::ip::make_address(_s.interface_address.data());
    auto const port = _s.interface_port;
    auto const doc_root = std::make_shared<string>(_s.document_root);

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12};

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);

    // Create and launch a listening port
    std::make_shared<listener>(
        *_ioc,
        ctx,
        boost::asio::ip::tcp::endpoint{address, port},
        doc_root)->run();

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
   * Calls stop on the io context.  Returns no value, does not block.  Success or failure is returned from the service::start() method
   * @see: boost::asio::io_context::stop
   */
  void stop() {
    _ioc->stop();
  }
};
}
