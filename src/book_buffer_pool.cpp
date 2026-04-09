#include <book_buffer_pool.hpp>

BookBufferPool::BookBufferPool(size_t chunk_size, size_t num_chunks) : chunk_size_(chunk_size), num_chunks_(num_chunks) {
    for (size_t i = 0; i < num_chunks_; i++) {
        auto mem = std::make_unique<BookUpdate[]>(chunk_size_);
        free_buffers_.push({mem.get(), chunk_size_, 0});
        chunks_.push_back(std::move(mem));
    }
}

BookUpdateBuffer BookBufferPool::acquire() {
    if (free_buffers_.empty()) throw std::runtime_error("No free buffers");
    BookUpdateBuffer buf = free_buffers_.top();
    free_buffers_.pop();
    buf.used = 0;
    return buf;
}

void BookBufferPool::release(BookUpdateBuffer buf) {
    free_buffers_.push(buf);
}
