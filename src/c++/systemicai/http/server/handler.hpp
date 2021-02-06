#ifndef SYSTEMICAI_HTTP_SERVER_HANDLER
#define SYSTEMICAI_HTTP_SERVER_HANDLER

#include "settings.h"
#include <boost/beast/http.hpp>

using namespace std;
namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>

template< class Body, class Allocator, class Send >
class handler {
public:
    bool handles(http::request<Body, http::basic_fields<Allocator>>& req) = 0;
};


#endif // SYSTEMICAI_HTTP_SERVER_HANDLER
