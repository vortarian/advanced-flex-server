//
// Created by vorta on 2/6/2021.
//
// This file is included from tst/c++/systemicai/unit_tests.cpp

#include <sstream>
using namespace std;
  using namespace systemicai::http::server;
  namespace pt = ::boost::property_tree;
  namespace ssl = ::boost::asio::ssl;

  void run_service(service& s, std::timed_mutex& tms) {
    try {
      std::lock_guard lg(tms);
      s.start();
    } catch(const std::exception& e) {
      std::cerr << "std::exception caught from service.start(): " << e.what() << std::endl;
    } catch(...) {
      std::cerr << "Unknown exception caught from service.start()";
    }
  }

  const string dummy_ssl_certificate(
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
    "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
    "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
    "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
    "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
    "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
    "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
    "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
    "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
    "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
    "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
    "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
    "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
    "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
    "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
    "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
    "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
    "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
    "UEVbkhd5qstF6qWK\n"
    "-----END CERTIFICATE-----");

  const string dummy_ssl_key("-----BEGIN PRIVATE KEY-----\n"
    "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
    "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
    "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
    "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
    "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
    "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
    "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
    "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
    "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
    "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
    "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
    "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
    "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
    "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
    "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
    "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
    "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
    "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
    "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
    "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
    "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
    "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
    "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
    "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
    "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
    "pVOUFq5mW8p0zbtfHbjkgxyF\n"
    "-----END PRIVATE KEY-----");

  const string dummy_ssl_dh("-----BEGIN DH PARAMETERS-----\n"
    "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
    "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
    "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
    "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
    "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
    "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
    "-----END DH PARAMETERS-----");

  // The test case must be registered with the test runner
  BOOST_AUTO_TEST_CASE( test_systemicai_http_server_service )
  {
    boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::trace );

    std::stringstream s_json(test::systemicai::http::server::settings::json);
    pt::ptree tree;
    pt::json_parser::read_json(s_json, tree);
    systemicai::http::server::settings settings(tree);
    ssl::context ssl_ctx{ssl::context::tlsv12};
    std::istringstream idsc(dummy_ssl_certificate);
    std::istringstream idsk(dummy_ssl_key);
    std::istringstream idsd(dummy_ssl_dh);
    systemicai::common::certificate::load(
      ssl_ctx,
      idsc,
      idsk,
      idsd
    );
    ssl_http_session::type_handler_registry r_ssl;
    plain_http_session::type_handler_registry r_plain;
    systemicai::http::server::service service(settings, ssl_ctx, r_ssl, r_plain);

    std::timed_mutex tms;
    BOOST_TEST(service.running() == false);
    std::thread t(run_service, std::ref(service), std::ref(tms));

    // Give the service 1 second to start
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    BOOST_TEST(service.running() == true);
    service.stop();

    // Try to acquire the lock for 3 seconds - giving the service 3 seconds to stop
    if(tms.try_lock_for(std::chrono::seconds(3))) {
      // If we acquire the lock, unlock it
      tms.unlock();
    }
    t.join();

    // We should be in a stopped state by now
    BOOST_TEST(service.running() == false);

    std::cout << "This test doesn't actually do anything yet - STUB only atm";
  }

  // The test case must be registered with the test runner
  BOOST_AUTO_TEST_CASE( test_systemicai_default_handler )
  {
    boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::trace );

    std::stringstream s_json(test::systemicai::http::server::settings::json);
    pt::ptree tree;
    pt::json_parser::read_json(s_json, tree);
    systemicai::http::server::settings settings(tree);
    ssl::context ssl_ctx{ssl::context::tlsv12};
    std::istringstream idsc(dummy_ssl_certificate);
    std::istringstream idsk(dummy_ssl_key);
    std::istringstream idsd(dummy_ssl_dh);
    systemicai::common::certificate::load(
      ssl_ctx,
      idsc,
      idsk,
      idsd
    );
    ssl_http_session::type_handler_registry r_ssl;
    plain_http_session::type_handler_registry r_plain;
    systemicai::http::server::service service(settings, ssl_ctx, r_ssl, r_plain);

    std::timed_mutex tms;
    BOOST_TEST(service.running() == false);
    std::thread t(run_service, std::ref(service), std::ref(tms));

    // Give the service 1 second to start
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    BOOST_TEST(service.running() == true);
    service.stop();

    // Try to acquire the lock for 3 seconds - giving the service 3 seconds to stop
    if(tms.try_lock_for(std::chrono::seconds(3))) {
      // If we acquire the lock, unlock it
      tms.unlock();
    }
    t.join();

    // We should be in a stopped state by now
    BOOST_TEST(service.running() == false);
  }

  /**
  * This template function sets up all the handlers for the test_systemicai_register_handler
  */
  template<class Registry> void test_systemicai_register_handler_register_test_handlers(Registry& r) {
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
        boost::urls::url u(req_.target());
        for(auto param = u.params().find("status"); param != u.params().end(); param++) {
          res.result(std::atoi(param->value().c_str()));
        }
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
        res.set(beast::http::field::server, s.service_version);
        res.body() = sstr.str();
        res.prepare_payload();
        send(std::move(res));
      }
    };
    r.addHandler(std::make_shared<h>());
  }

  // The test case must be registered with the test runner
  BOOST_AUTO_TEST_CASE( test_systemicai_register_handler )
  {
    boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::trace );

    std::stringstream s_json(test::systemicai::http::server::settings::json);
    pt::ptree tree;
    pt::json_parser::read_json(s_json, tree);
    systemicai::http::server::settings s(tree);
    std::istringstream idsc(dummy_ssl_certificate);
    std::istringstream idsk(dummy_ssl_key);
    std::istringstream idsd(dummy_ssl_dh);
    ssl::context ssl_ctx{ssl::context::tlsv12};
    systemicai::common::certificate::load(
      ssl_ctx,
      idsc,
      idsk,
      idsd
    );

    ssl_http_session::type_handler_registry r_ssl;
    plain_http_session::type_handler_registry r_plain;
    test_systemicai_register_handler_register_test_handlers(r_ssl);
    test_systemicai_register_handler_register_test_handlers(r_plain);
    systemicai::http::server::service service(s, ssl_ctx, r_ssl, r_plain);

    std::timed_mutex tms;
    std::thread t(run_service, std::ref(service), std::ref(tms));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    {
      auto resp = simple_client_get<beast::http::dynamic_body>("localhost", boost::lexical_cast<std::string>(s.interface_port), "/echo?status=503", 11) ;
      BOOST_TEST(resp.result_int() == 503);
    }

    {
      auto resp = simple_client_get<beast::http::dynamic_body>("localhost", boost::lexical_cast<std::string>(s.interface_port), "/echo?status=200", 11) ;
      BOOST_TEST(resp.result_int() == 200);
    }

    // SSL Tests
    idsc = std::istringstream(dummy_ssl_certificate);
    idsk = std::istringstream(dummy_ssl_key);
    idsd = std::istringstream(dummy_ssl_dh);
    ssl::context client_ssl_ctx{ssl::context::tlsv12};
    systemicai::common::certificate::load(
      client_ssl_ctx,
      idsc,
      idsk,
      idsd
    );

    {
      auto resp = ssl_client_get<beast::http::dynamic_body>(client_ssl_ctx, "localhost", boost::lexical_cast<std::string>(s.interface_port), "/echo?status=503", 11) ;
      BOOST_TEST(resp.result_int() == 503);
    }

    {
      auto resp = ssl_client_get<beast::http::dynamic_body>(client_ssl_ctx, "localhost", boost::lexical_cast<std::string>(s.interface_port), "/echo?status=200", 11) ;
      BOOST_TEST(resp.result_int() == 200);
    }

    service.stop();
    // Try to acquire the lock for 3 seconds - giving the service 3 seconds to stop
    if(tms.try_lock_for(std::chrono::seconds(3))) {
      // If we acquire the lock, unlock it
      tms.unlock();
    }
    t.join();

    // We should be in a stopped state by now
    BOOST_TEST(service.running() == false);
  }