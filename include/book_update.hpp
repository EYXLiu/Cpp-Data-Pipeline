#ifndef BOOK_UPDATE_HPP
#define BOOK_UPDATE_HPP
#include <stdint.h>

struct BookUpdate {
    double price;
    double qty;
    bool is_bid;
    uint64_t ts;
};

#endif
