#ifndef BOOK_UPDATE_HPP
#define BOOK_UPDATE_HPP
#include <stdint.h>

struct BookUpdate {
    double price;
    double qty;
    bool is_bid;
    
    uint64_t t_ingest;
    uint64_t t_parsed;
    uint64_t t_processed;
};

#endif
