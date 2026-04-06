#include "kraken_ingestor.hpp"
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <simdjson.h>
#include <iostream>

KrakenDataIngestor::KrakenDataIngestor(Callback cb) : callback_(cb) {}

void KrakenDataIngestor::start() {
    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    namespace ssl = net::ssl;
    using tcp = net::ip::tcp;

    net::io_context ioc;
    ssl::context ctx(ssl::context::tlsv12_client);

    tcp::resolver resolver(ioc);
    auto results = resolver.resolve("ws.kraken.com", "443");

    websocket::stream<ssl::stream<tcp::socket>> ws(ioc, ctx);

    net::connect(ws.next_layer().next_layer(), results);
    
    if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), "ws.kraken.com")) {
        boost::system::error_code ec{
            static_cast<int>(::ERR_get_error()),
            boost::asio::error::get_ssl_category()};
        throw boost::system::system_error{ec};
    }

    ws.next_layer().handshake(ssl::stream_base::client);
    ws.handshake("ws.kraken.com", "/");

    std::string sub = R"({
        "event":"subscribe",
        "pair":["XBT/USD"],
        "subscription":{"name":"book","depth":10}
    })";

    ws.write(net::buffer(sub));

    beast::flat_buffer buffer;

    while (true) {
        ws.read(buffer);

        std::string msg = beast::buffers_to_string(buffer.data());
        handle_message(msg);

        buffer.consume(buffer.size());
    }
}

uint64_t KrakenDataIngestor::now_ns() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void KrakenDataIngestor::handle_message(const std::string& msg) {
    using namespace simdjson;
    ondemand::parser parser;

    uint64_t ingested = now_ns();
    try {
        padded_string padded(msg.data(), msg.size());

        auto j = parser.iterate(padded);
        ondemand::array top = j.get_array();

        auto it = top.begin();
        ++it;

        ondemand::object data = (*it).get_object();

        ondemand::array bids;
        if (data["b"].get_array().get(bids) == SUCCESS) {
            for (ondemand::array bid : bids) {
                auto it = bid.begin();

                std::string_view price_str = (*it).get_string();
                ++it;
                std::string_view qty_str = (*it).get_string();

                BookUpdate u;
                u.price = std::stod(std::string(price_str));
                u.qty   = std::stod(std::string(qty_str));
                u.is_bid = true;
                u.t_ingest = ingested;
                u.t_parsed = now_ns();

                callback_(u);
            }
        }

        ondemand::array asks;
        if (data["a"].get_array().get(asks) == SUCCESS) {
            for (ondemand::array ask : asks) {
                auto it = ask.begin();

                std::string_view price_str = (*it).get_string();
                ++it;
                std::string_view qty_str = (*it).get_string();

                BookUpdate u;
                u.price = std::stod(std::string(price_str));
                u.qty   = std::stod(std::string(qty_str));
                u.is_bid = false;
                u.t_ingest = ingested;
                u.t_parsed = now_ns();

                callback_(u);
            }
        }
    }
    // catch (const std::exception& e) {
    //     std::cerr << "handle_message error: " << e.what() << "\n";
    // }
    catch (...) {

    }
}
