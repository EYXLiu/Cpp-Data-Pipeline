#ifndef BOOK_UPDATE_HPP
#define BOOK_UPDATE_HPP
#include <stdint.h>
#include <string>

struct BookUpdate {
    std::string price;
    std::string qty;
    bool is_bid;
    
    uint64_t t_ingest;
    uint64_t t_parsed;
    uint64_t t_processed;
};

#endif
