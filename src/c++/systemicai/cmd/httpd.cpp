//------------------------------------------------------------------------------

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <boost/property_tree/json_parser.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <systemicai/http/server/service.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

using namespace systemicai::http::server;
using namespace std;

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

int main(int argc, char* argv[])
{
  // Check command line arguments.
  if (argc != 2)
  {
    std::cerr <<
              "Usage: " << argv[0] << " settings.json\n" <<
              "Example:\n" <<
              "    " << argv[0] << " cfg/settings.json\n";
    return EXIT_FAILURE;
  }

  pt::ptree tree;
  pt::json_parser::read_json(argv[1], tree);
  settings &g(settings::globals());
  set_log_filter(g);
  g.load(tree);

  ssl::context ssl_ctx{ssl::context::tlsv12};
  systemicai::common::certificate::load(ssl_ctx, g.ssl_certificate, g.ssl_key, g.ssl_dh);
  int status = 0;
  try{
    systemicai::http::server::service httpd(g, ssl_ctx);
    status = httpd.start();
  } catch(const char* msg) {
    std::cerr << "Uncaught exception: " << msg << std::endl;
    status = 1;
  } catch(const std::exception& e) {
    std::cerr << "Uncaught exception: " << e.what() << std::endl;
    status = 2;
  }
  return status;
}
