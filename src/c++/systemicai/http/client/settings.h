//
// Created by vortarian on 2021/01/19.
//

#ifndef SYSTEMICAI_HTTP_CLIENT_SETTINGS_H
#define SYSTEMICAI_HTTP_CLIENT_SETTINGS_H


#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/chrono.hpp>

namespace systemicai {
namespace http {
namespace client {

using std::string;
using std::size_t;
namespace pt = boost::property_tree;

struct settings {
  string interface_address;
  unsigned short interface_port;
  string log_level;
  string ssl_certificate;
  string ssl_key;
  string ssl_dh;
  int thread_io;
  size_t timeout_header;
  size_t timeout_get;
  size_t timeout_put;
  size_t timeout_post;
  uint64_t limit_history;

  settings() = default;

  settings(const pt::ptree &tr) {
    load(tr);
  }

  void load(const pt::ptree &tr) {
    log_level = tr.get<string>("service.log.level", "info");
    boost::algorithm::to_lower(log_level);
    ssl_certificate = tr.get<string>("service.ssl.certificate", "cfg/dumb.cert");
    ssl_key = tr.get<string>("service.ssl.key", "cfg/dumb.key");
    ssl_dh = tr.get<string>("service.ssl.dh", "cfg/dumb.dh");
    thread_io = tr.get<int>("service.thread.io", 1);
    timeout_header = tr.get<>("service.timeout.header", 5);
    timeout_get = tr.get<size_t>("service.timeout.get", 300);
    timeout_put = tr.get<size_t>("service.timeout.put", 300);
    timeout_post = tr.get<size_t>("service.timeout.post", 300);
  }

  operator pt::ptree() {
    pt::ptree tr;
    tr.put("service.log.level", log_level);
    tr.put("service.ssl.certificate", ssl_certificate);
    tr.put("service.ssl.key", ssl_key);
    tr.put("service.ssl.dh", ssl_dh);
    tr.put("service.thread.io", thread_io);
    tr.put("service.timeout.header", timeout_header);
    tr.put("service.timeout.get", timeout_get);
    tr.put("service.timeout.put", timeout_put);
    tr.put("service.timeout.post", timeout_post);
    return tr;
  }

};
}
}
}

#endif // SYSTEMICAI_HTTP_CLIENT_SETTINGS_H
