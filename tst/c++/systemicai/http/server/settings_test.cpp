//
// Created by vorta on 2/6/2021.
//
// This file is included from tst/c++/systemicai/unit_tests.cpp

// This macro must be called before including the header

#include <systemicai/http/server/namespace.h>
#include <boost/test/included/unit_test.hpp>

namespace pt = boost::property_tree;

// The test case must be registered with the test runner
BOOST_AUTO_TEST_CASE( test_systemicai_http_server_settings )
{
  boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::trace );
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
BOOST_AUTO_TEST_CASE( test_systemicai_http_server_settings_json_parsing )
{
  std::stringstream s_json(test::systemicai::http::server::settings::json);
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
  BOOST_TEST(settings.thread_io == 2);
}

