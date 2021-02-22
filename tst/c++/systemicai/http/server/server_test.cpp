//
// Created by vorta on 2/6/2021.
//
// This file is included from tst/c++/systemicai/unit_tests.cpp

namespace pt = boost::property_tree;

// The test case must be registered with the test runner
  BOOST_AUTO_TEST_CASE( test_systemicai_http_server_service )
  {
    std::stringstream s_json(test::systemicai::http::server::settings::json);
    pt::ptree tree;
    pt::json_parser::read_json(s_json, tree);
    systemicai::http::server::settings settings(tree);
    systemicai::http::server::service service(settings);

    // STUB: This is only so there is a test in this stub
    BOOST_TEST(settings.log_level == "debug");

    BOOST_LOG_TRIVIAL(warning) << "This test doesn't actually do anything yet - STUB only atm";
  }