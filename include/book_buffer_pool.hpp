#ifndef BOOK_BUFFER_POOL_HPP
#define BOOK_BUFFER_POOL_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <stack>
#include <memory>
#include "book_update.hpp"

struct BookUpdateBuffer { 
    BookUpdate* data;
    size_t size;
    size_t used;
};

class BookBufferPool {
public:
    BookBufferPool(size_t chunk_size, size_t num_chunks);

    BookUpdateBuffer acquire();
    void release(BookUpdateBuffer buf);
private:
    std::vector<std::unique_ptr<BookUpdate[]>> chunks_;
    std::stack<BookUpdateBuffer> free_buffers_;
    size_t chunk_size_;
    size_t num_chunks_;
};

#endif
