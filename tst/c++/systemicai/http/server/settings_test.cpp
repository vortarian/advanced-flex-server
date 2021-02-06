//
// Created by vorta on 2/6/2021.
//

// This macro must be called before including the header
#define BOOST_TEST_MODULE test-http-server-settings
#include <boost/test/included/unit_test.hpp>

#include <cstddef>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <boost/property_tree/json_parser.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <systemicai/http/server/settings.h>

namespace pt = boost::property_tree;

static const char* test_json(R"(
  {
    "document": {
      "root": "./document-root"
    },
    "service": {
      "interface": {
        "address": "0.0.0.0",
        "port": 8080
      },
      "log": {
        "level": "debug"
      },
      "ssl": {
        "certificate": "cfg/dumb.cert",
        "key": "cfg/dumb.key",
        "dh": "cfg/dumb.dh"
      },
      "timeout": {
        "header": "15000",
        "put": "3000",
        "post": "3000",
        "get": "10000"
      },
      "thread": {
        "io": "22"
      }
    }
  }
  )");

// The test case must be registered with the test runner
BOOST_AUTO_TEST_CASE( test_http_server_settings_global )
{
  systemicai::http::server::settings& settings(systemicai::http::server::settings::globals());
  systemicai::http::server::settings* null_ptr_settings(nullptr);
  // Be sure it isn't a null reference
  BOOST_TEST(&settings != null_ptr_settings);

  // Check for a default value to see that things parsed
  BOOST_TEST(settings.log_level == "info");

  // The returned object should be a global singleton, so it shouldn't change between invocations
  systemicai::http::server::settings& settings_1(systemicai::http::server::settings::globals());
  systemicai::http::server::settings& settings_2(systemicai::http::server::settings::globals());
  BOOST_TEST( (&settings_1 == &settings_2 && &settings_2 == &settings ) );

  // Globals should be modifiable
  int thread_io = settings.thread_io;
  settings.thread_io++;
  BOOST_TEST(settings_2.thread_io == (thread_io + 1));
}

// The test case must be registered with the test runner
BOOST_AUTO_TEST_CASE( test_http_server_settings_json_parsing)
{
  std::stringstream s_json(test_json);
  pt::ptree tree;
  pt::json_parser::read_json(s_json, tree);
  systemicai::http::server::settings settings(tree);
  BOOST_TEST(settings.log_level == "debug");
  BOOST_TEST(settings.document_root == "./document-root");
  BOOST_TEST(settings.interface_address == "0.0.0.0");
  BOOST_TEST(settings.interface_port == 8080);
  BOOST_TEST(settings.log_level == "debug");
  BOOST_TEST(settings.ssl_certificate == "cfg/dumb.cert");
  BOOST_TEST(settings.ssl_key == "cfg/dumb.key");
  BOOST_TEST(settings.ssl_dh == "cfg/dumb.dh");
  BOOST_TEST(settings.timeout_header == 15000);
  BOOST_TEST(settings.timeout_put == 3000);
  BOOST_TEST(settings.timeout_post == 3000);
  BOOST_TEST(settings.timeout_get == 10000);
  BOOST_TEST(settings.thread_io == 22);
}


