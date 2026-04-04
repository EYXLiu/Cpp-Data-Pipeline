#include "kraken_ingestor.hpp"
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <regex>
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
    uint64_t ingested = now_ns();

    std::regex block_re(R"REGEX("([ab])"\s*:\s*(\[[^\]]+\]))REGEX");
    std::regex entry_re(
        R"REGEX(\["([0-9.]+)","([0-9.]+)","[0-9.]+"(?:,"r")?\])REGEX"
    );

    auto block_begin = std::sregex_iterator(msg.begin(), msg.end(), block_re);
    auto block_end = std::sregex_iterator();

    for (auto it = block_begin; it != block_end; ++it) {
        std::smatch block_match = *it;

        bool is_bid = (block_match[1] == "b");
        std::string entries = block_match[2];

        auto entry_begin = std::sregex_iterator(entries.begin(), entries.end(), entry_re);
        auto entry_end = std::sregex_iterator();

        uint64_t parsed = now_ns();

        for (auto jt = entry_begin; jt != entry_end; ++jt) {
            std::smatch m = *jt;

            BookUpdate u;
            u.price = std::stod(m[1].str());
            u.qty   = std::stod(m[2].str());
            u.is_bid = is_bid;
            u.t_ingest = ingested;
            u.t_parsed = parsed;

            callback_(u);
        }
    }
}
