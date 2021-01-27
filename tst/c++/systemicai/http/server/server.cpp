//------------------------------------------------------------------------------

#include <systemicai/http/server/server.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <boost/property_tree/json_parser.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS

using namespace systemicai::http::server;
using namespace std;

//------------------------------------------------------------------------------
void load_server_certificate(boost::asio::ssl::context& ctx, const settings& s)
{
    ifstream cs(s.ssl_certificate), ks(s.ssl_key), ds(s.ssl_dh);

    std::string const cert(istreambuf_iterator<char>{cs}, {});
    std::string const key(istreambuf_iterator<char>{ks}, {});
    std::string const dh(istreambuf_iterator<char>{ds}, {});

    ctx.set_password_callback(
            [](std::size_t,
               boost::asio::ssl::context_base::password_purpose)
            {
                return "test";
            });

    ctx.set_options(
            boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::single_dh_use);

    ctx.use_certificate_chain(
            boost::asio::buffer(cert.data(), cert.size()));

    ctx.use_private_key(
            boost::asio::buffer(key.data(), key.size()),
            boost::asio::ssl::context::file_format::pem);

    ctx.use_tmp_dh(
            boost::asio::buffer(dh.data(), dh.size()));
}

bool set_log_filter(const settings& s) {
    if(s.log_level == "trace") {
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::trace
        );
    } else if(s.log_level == "info") {
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::info
        );
    } else if(s.log_level == "debug") {
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::debug
        );
    } else if(s.log_level == "warning") {
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::warning
        );
    } else if(s.log_level == "error") {
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::error
        );
    } else if(s.log_level == "fatal") {
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::fatal
        );
    } else {
        BOOST_LOG_TRIVIAL(fatal) << "Unknown log level specified: [" << s.log_level << "] valid values are [trace, info, debug, warrning, error, fatal]";
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 2)
    {
        std::cerr <<
                  "Usage: " << argv[0] << " settings.json\n" <<
                  "Example:\n" <<
                  "    " << argv[0] << " cfg/settings.json\n";
        return EXIT_FAILURE;
    }

    pt::ptree tree;
    pt::json_parser::read_json(argv[1], tree);
    settings s(tree);

    auto const address = net::ip::make_address(s.interface_address.data());
    auto const port = s.interface_port;
    auto const doc_root = std::make_shared<string>(s.document_root);
    auto const threads = std::max<int>(1, s.thread_io);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12};

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx, s);

    // Create and launch a listening port
    std::make_shared<listener>(
            ioc,
            ctx,
            tcp::endpoint{address, port},
            doc_root)->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
            [&](beast::error_code const&, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                ioc.stop();
            });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    // Block until all the threads exit
    for(auto& t : v)
        t.join();

    return EXIT_SUCCESS;
}