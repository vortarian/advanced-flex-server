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
#include <systemicai/http/server/service.hpp>

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

BOOST_AUTO_TEST_SUITE(test_http_server_service)

// The test case must be registered with the test runner
  BOOST_AUTO_TEST_CASE( global )
  {
    std::stringstream s_json(test_json);
    pt::ptree tree;
    pt::json_parser::read_json(s_json, tree);
    systemicai::http::server::settings settings(tree);
    systemicai::http::server::service service(settings);

    // STUB: This is only so there is a test in this stub
    BOOST_TEST(settings.log_level == "debug");

    BOOST_LOG_TRIVIAL(warning) << "This test doesn't actually do anything yet - STUB only atm";
  }

BOOST_AUTO_TEST_SUITE_END()
