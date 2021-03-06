
#pragma once

namespace test::systemicai::http::server::settings {

static const char* json(R"(
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

}
