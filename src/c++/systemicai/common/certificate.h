//
// Created by vortarian on 2021/01/19.
//

#ifndef SYSTEMICAI_COMMON_CERTIFICATE_H
#define SYSTEMICAI_COMMON_CERTIFICATE_H

#include <systemicai/http/client/settings.h>
#include <boost/asio/ssl.hpp>
#include <streambuf>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <iostream>

namespace systemicai {
namespace common {

using namespace std;

class certificate {
public:
  /**
   * Load a certificate from the given path and set it to the ASIO context
   * @param ctx The ASIO context to put the certificate in
   * @param ssl_certificate The path to the SSL certificate
   * @param ssl_key The path to the SSL key
   * @param ssl_dh The path to the diffie hellman data
   */
  static void load(
      boost::asio::ssl::context& ctx,
      const std::filesystem::path& ssl_certificate,
      const std::filesystem::path& ssl_key,
      const std::filesystem::path& ssl_dh
      ) {
    std::filesystem::path f_cs(ssl_certificate);
    ifstream cs(f_cs);
    if(!cs) {
      throw std::filesystem::filesystem_error("Not Found", f_cs, std::make_error_code(std::errc::io_error));
    }

    std::filesystem::path f_ks(ssl_key);
    ifstream ks(f_ks);
    if(!ks) {
      throw std::filesystem::filesystem_error("Not Found", f_ks, std::make_error_code(std::errc::io_error));
    }

    std::filesystem::path f_ds(ssl_dh);
    ifstream ds(f_ds);
    if(!ds) {
      throw std::filesystem::filesystem_error("Not Found", f_ds, std::make_error_code(std::errc::io_error));
    }
    load(ctx, cs, ks, ds);
  }

  /**
   * Load a certificate from the given path and set it to the ASIO context
   * @param ctx The ASIO context to put the certificate in
   * @param s The settings object which contains information about the location of the certificate data
   */
  static void load(
      boost::asio::ssl::context& ctx,
      std::istream& istream_ssl_certificate,
      std::istream& istream_ssl_key,
      std::istream& istream_ssl_dh)
  {
    ctx.set_password_callback([](std::size_t, boost::asio::ssl::context_base::password_purpose) {
        return "Not Implemented";
    });

    ctx.set_options(
      boost::asio::ssl::context::default_workarounds |
      boost::asio::ssl::context::no_sslv2 |
      boost::asio::ssl::context::single_dh_use);

    if(!istream_ssl_certificate) {
      throw std::system_error(std::make_error_code(std::errc::io_error), "istream_ssl_certificate is invalid");
    }
    std::string const cert(istreambuf_iterator<char>{istream_ssl_certificate}, {});
    ctx.use_certificate_chain(boost::asio::buffer(cert.data(), cert.size()));

    if(!istream_ssl_key) {
      throw std::system_error(std::make_error_code(std::errc::io_error), "istream_ssl_key is invalid");
    }
    std::string const key(istreambuf_iterator<char>{istream_ssl_key}, {});
    ctx.use_private_key(
      boost::asio::buffer(key.data(), key.size()),
      boost::asio::ssl::context::file_format::pem
    );

    if(!istream_ssl_dh) {
      throw std::system_error(std::make_error_code(std::errc::io_error), "istream_ssl_dh is invalid");
    }
    std::string const dh(istreambuf_iterator<char>{istream_ssl_dh}, {});
    ctx.use_tmp_dh(boost::asio::buffer(dh.data(), dh.size()));
  }
};

}
}

#endif // SYSTEMICAI_COMMON_CERTIFICATE_H
