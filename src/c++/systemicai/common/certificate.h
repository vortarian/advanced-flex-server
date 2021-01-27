//
// Created by vortarian on 2021/01/19.
//

#ifndef SYSTEMICAI_COMMON_CERTIFICATE_H
#define SYSTEMICAI_COMMON_CERTIFICATE_H

#include <systemicai/http/client/settings.h>
#include <boost/asio/ssl.hpp>
#include <streambuf>
#include <fstream>


namespace systemicai {
namespace http {
namespace client {

using namespace std;

class certificate {
public:
  /**
   * Load a certificate from the given path and set it to the ASIO context
   * @param ctx The ASIO context to put the certificate in
   * @param s The settings object which contains information about the location of the certificate data
   */
  static void load(boost::asio::ssl::context& ctx, const settings& s)
  {


    ctx.set_password_callback([](std::size_t, boost::asio::ssl::context_base::password_purpose) {
        return "Not Implemented";
    });

    ctx.set_options(
      boost::asio::ssl::context::default_workarounds |
      boost::asio::ssl::context::no_sslv2 |
      boost::asio::ssl::context::single_dh_use);

    ifstream cs(s.ssl_certificate);
    std::string const cert(istreambuf_iterator<char>{cs}, {});
    ctx.use_certificate_chain(
      boost::asio::buffer(cert.data(), cert.size()));

    ifstream ks(s.ssl_key);
    std::string const key(istreambuf_iterator<char>{ks}, {});
    ctx.use_private_key(
      boost::asio::buffer(key.data(), key.size()),
      boost::asio::ssl::context::file_format::pem);

    ifstream ds(s.ssl_dh);
    std::string const dh(istreambuf_iterator<char>{ds}, {});
    ctx.use_tmp_dh(
      boost::asio::buffer(dh.data(), dh.size()));
  }
};

}
}
}

#endif // SYSTEMICAI_COMMON_CERTIFICATE_H
