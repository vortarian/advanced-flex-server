//------------------------------------------------------------------------------

#include "systemicai/http/server/sessions.hpp"
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

/**
 * This template function sets up all the handlers for the service behind the default handlers
 */
template<class Registry> void register_handlers(Registry& r) {
  struct h : public handlers::Handler<typename Registry::type_fields, typename Registry::type_send>
  {
    h() : handlers::Handler<typename Registry::type_fields, typename Registry::type_send>() {}

    bool handles(const beast::http::request_header<typename Registry::type_fields> &req, const settings& s) const {

      if(req.target() == "/echo" || req.target().starts_with("/echo?")) return true;
      else return false;
    }

    void handle(const beast::http::request_header<typename Registry::type_fields> &req, typename Registry::type_send& send, const settings& s) const {
      beast::http::request<beast::http::string_body, typename Registry::type_fields> req_(req);
      beast::http::response<beast::http::string_body> res{beast::http::status::ok, req.version()};
      res.set(beast::http::field::server, s.service_version);
      res.set(beast::http::field::content_type, "text/html");
      res.keep_alive(req_.keep_alive());

      std::stringstream sstr;
      sstr << "<html><body>";
      sstr << "<br/><h2>Target</h2><br>";
      sstr << req_.target();
      sstr << "<br/><h2>Headers</h2><br><ul>";
      for(auto hdr = req_.base().begin(); hdr != req_.base().end(); hdr++) {
        sstr << "<li><span>" << hdr->name_string() << "</span>&nbsp;=&nbsp;<span>" << hdr->value() << "</span>";
      }
      sstr << "</ul><br/><h2>Body</h2><br/>";
      sstr << req_.body() << "</body></html>";
      res.body() = sstr.str();
      res.prepare_payload();
      send(std::move(res));
    }
  };
  r.addHandler(std::make_shared<h>());
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
  ssl_http_session::type_handler_registry registry_handler_ssl;
  plain_http_session::type_handler_registry registry_handler_plain;

  register_handlers(registry_handler_ssl);
  register_handlers(registry_handler_plain);

  try {
    systemicai::http::server::service httpd(g, ssl_ctx, registry_handler_ssl, registry_handler_plain);
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
