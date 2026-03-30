#ifndef KRAKEN_INGESTOR_HPP
#define KRAKEN_INGESTOR_HPP
#include "book_update.hpp"
#include <functional>

class KrakenDataIngestor {
public:
    using Callback = std::function<void(const BookUpdate&)>;

    KrakenDataIngestor(Callback cb);
    void start();
private:
    Callback callback_;
    void handle_message(const std::string& msg);
    uint64_t now_ns();
};

#endif
