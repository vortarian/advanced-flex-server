//
// Created by vortarian on 2021/01/19.
//

#include <systemicai/http/client/api.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <boost/property_tree/json_parser.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <network/uri.hpp>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
using namespace systemicai::http::client;
using namespace std;
using namespace boost::placeholders;

//------------------------------------------------------------------------------
void load_server_certificate(boost::asio::ssl::context& ctx, const settings& s)
{
  ifstream cs(s.ssl_certificate), ks(s.ssl_key), ds(s.ssl_dh);

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

bool set_log_filter(const settings& s) {
  if(s.log_level == "trace") {
    boost::log::core::get()->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::trace
    );
  } else if(s.log_level == "info") {
    boost::log::core::get()->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::info
    );
  } else if(s.log_level == "debug") {
    boost::log::core::get()->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::debug
    );
  } else if(s.log_level == "warning") {
    boost::log::core::get()->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::warning
    );
  } else if(s.log_level == "error") {
    boost::log::core::get()->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::error
    );
  } else if(s.log_level == "fatal") {
    boost::log::core::get()->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::fatal
    );
  } else {
    BOOST_LOG_TRIVIAL(fatal) << "Unknown log level specified: [" << s.log_level << "] valid values are [trace, info, debug, warrning, error, fatal]";
    return false;
  }
  return true;
}
int main(int argc, char **argv) {
  // Check command line arguments.
  if (argc != 4 && argc != 5) {
    std::cerr <<
              "Usage: " << argv[0] << " settings.json <uri> <threads> <version>\n" <<
              "Example:\n" <<
              "   " << argv[0] << " settings.json https://www.example.com:443/somepath <concurrency = 2> <http version = 1.0>\n";
    return EXIT_FAILURE;
  }

  pt::ptree tree;
  pt::json_parser::read_json(argv[1], tree);
  settings s(tree);

  set_log_filter(s);

  string const url = argv[2];
  network::uri u(url);
  int concurrency = argc == 4 ? std::atoi(argv[3]) : s.thread_io;
  string port = u.has_port() ? u.port().to_string() : "443";

  int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;
  // The io_context is required for all I/O
  boost::asio::io_context ioc;

  // The SSL context is required, and holds certificates
  ssl::context ctx{ssl::context::sslv23_client};

  // This holds the root certificate used for verification
  load_server_certificate(ctx, s);

  std::list<std::shared_ptr<signing>> tokenization(1);
  // Launch the asynchronous operation
  tokenization.push_back(std::make_shared<signing>(ioc, ctx));
  tokenization.back()->run(
      u.host().to_string().data(),
      port.data(),
      u.path().to_string().data(),
      version
  );

  // Run the I/O service. The call will return when
  // the get operation is complete.
  ioc.run();

  return EXIT_SUCCESS;
}

