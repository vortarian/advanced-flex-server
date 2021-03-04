//
// Created by vorta on 2/22/2021.
//

// Unit Tests is a place to include all the cpp files for a test in a single location
// As this is a terrible design, a need to explain ..
// Code Coverage metrics by LLVM appear to be able to be calculated only on a single executable
// at a time, so linking all the unit tests to a single unit_test file makes it trivial to
// calculate an effective code coverage report in CICD that humans can easily grok.
// So this is a mono test file to keep it simple.  This will probably need to be revisited later.

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <boost/property_tree/json_parser.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS

#define BOOST_TEST_MODULE test-systemicai-http
#include <boost/test/included/unit_test.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <cstddef>
#include <systemicai/http/server/settings.h>
#include <systemicai/http/server/service.hpp>
#include <systemicai/http/server/server.h>

// All of our unit tests must be included between these two macros (and must not use these two macros)
BOOST_AUTO_TEST_SUITE(test_systemicai_http)

#include <systemicai/http/server/settings_test.hpp>
#include <systemicai/http/server/server_test.cpp>
#include <systemicai/http/server/settings_test.cpp>

BOOST_AUTO_TEST_SUITE_END()
