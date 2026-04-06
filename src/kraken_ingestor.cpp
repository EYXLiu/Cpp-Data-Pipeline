#include "kraken_ingestor.hpp"
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <rapidjson/document.h>
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
    using namespace rapidjson;

    uint64_t ingested = now_ns();

    try {
        Document doc;
        doc.Parse(msg.c_str());

        if (!doc.IsArray() || doc.Size() < 2) return;

        const Value& data = doc[1];
        if (!data.IsObject()) return;

        if (data.HasMember("b") && data["b"].IsArray()) {
            const Value& bids = data["b"];

            for (auto& bid : bids.GetArray()) {
                if (!bid.IsArray() || bid.Size() < 2)
                    continue;

                const char* price_str = bid[0].GetString();
                const char* qty_str   = bid[1].GetString();

                BookUpdate u;
                u.price = std::stod(price_str);
                u.qty   = std::stod(qty_str);
                u.is_bid = true;
                u.t_ingest = ingested;
                u.t_parsed = now_ns();

                callback_(u);
            }
        }

        if (data.HasMember("a") && data["a"].IsArray()) {
            const Value& asks = data["a"];

            for (auto& ask : asks.GetArray()) {
                if (!ask.IsArray() || ask.Size() < 2)
                    continue;

                const char* price_str = ask[0].GetString();
                const char* qty_str   = ask[1].GetString();

                BookUpdate u;
                u.price = std::stod(price_str);
                u.qty   = std::stod(qty_str);
                u.is_bid = true;
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
