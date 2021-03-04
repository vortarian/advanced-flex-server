#ifndef SYSTEMICAI_HTTP_SERVER_NAMESPACE_H
#define SYSTEMICAI_HTTP_SERVER_NAMESPACE_H

/**
  Common namespace definitions for this namespace
 */

#include <boost/asio.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/chrono.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>


namespace systemicai::http::server {
  namespace beast = boost::beast;       // from <boost/beast.hpp>
  namespace net = boost::asio;          // from <boost/asio.hpp>
  namespace ssl = boost::asio::ssl;     // from <boost/asio/ssl.hpp>
  namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>
  using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

}
#endif // SYSTEMICAI_HTTP_SERVER_NAMESPACE_H