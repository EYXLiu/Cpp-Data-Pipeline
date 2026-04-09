#ifndef KRAKEN_INGESTOR_HPP
#define KRAKEN_INGESTOR_HPP
#include "book_buffer_pool.hpp"
#include "book_update.hpp"
#include <functional>

class KrakenDataIngestor {
public:
    using Callback = std::function<void(BookUpdate&)>;

    KrakenDataIngestor(Callback cb, BookBufferPool& book_pool);
    void start();
    static uint64_t now_ns();
private:
    Callback callback_;
    BookBufferPool& book_pool_;
    void handle_message(const std::string& msg);
};

#endif
