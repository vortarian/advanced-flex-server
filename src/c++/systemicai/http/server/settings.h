#ifndef SYSTEMICAI_HTTP_SERVER_SETTINGS_H
#define SYSTEMICAI_HTTP_SERVER_SETTINGS_H

#include <systemicai/http/server/namespace.h>

namespace systemicai::http::server {

using std::string;
using std::size_t;
namespace pt = boost::property_tree;

struct settings {
    string interface_address;
    unsigned short interface_port;
    string document_root;
    string log_level;
    string service_version;
    string ssl_certificate;
    string ssl_key;
    string ssl_dh;
    int thread_io;
    size_t timeout_header;
    size_t timeout_get;
    size_t timeout_put;
    size_t timeout_post;

    /**
     * @param tr Property Tree with settings.  An empty tree is provided as the
     * default to allow default settings to be used.
     */
    settings(const pt::ptree &tr = pt::ptree()) {
        load(tr);
    }

    void load(const pt::ptree &tr) {
        interface_address = tr.get<string>("service.interface.address", "127.0.0.1");
        interface_port = tr.get<unsigned short>("service.interface.port", 8080);
        document_root = tr.get<string>("document.root", "html");
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
        service_version = tr.get<string>("service.version", "alpha");
    };

    operator pt::ptree() {
        pt::ptree tr;
        tr.put("service.interface.address", interface_address);
        tr.put("service.interface.port", interface_port);
        tr.put("document.root", document_root);
        tr.put("service.log.level", log_level);
        tr.put("service.ssl.certificate", ssl_certificate);
        tr.put("service.ssl.key", ssl_key);
        tr.put("service.ssl.dh", ssl_dh);
        tr.put("service.thread.io", thread_io);
        tr.put("service.timeout.header", timeout_header);
        tr.put("service.timeout.get", timeout_get);
        tr.put("service.timeout.put", timeout_put);
        tr.put("service.timeout.post", timeout_post);
        tr.put("service.version", service_version);
        return tr;
    }

    static settings& globals() {
        static struct settings s;
        return s;
    }
};

}

#endif // SYSTEMICAI_HTTP_SERVER_SETTINGS_H
