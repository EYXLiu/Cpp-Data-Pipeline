#include <connection.hpp>
#include <unistd.h>
#include <iostream>

Connection::Connection(int fd, BufferPool& pool, Reactor& reactor) : fd_(fd), write_size_(0), write_offset_(0), pool_(pool), reactor_(reactor) {
    try {
        read_buf_ = pool_.acquire();
    } catch (std::runtime_error e) {
        std::cout << "Failed to acquire read buffer" << std::endl;
        reactor_.schedule_close(fd_);
        return;
    }

    try {
        write_buf_ = pool_.acquire();
    } catch (std::runtime_error e) {
        std::cout << "Failed to acquire write buffer" << std::endl;
        pool.release(read_buf_);
        reactor_.schedule_close(fd_);
        return;
    }

    std::cout << "Client opened fd=" << fd << std::endl;
}

Connection::~Connection() {
    pool_.release(read_buf_);
    pool_.release(write_buf_);
    reactor_.schedule_close(fd_);
}

void Connection::on_readable() {
    ssize_t n = read(fd_, read_buf_.data + read_buf_.used, read_buf_.size - read_buf_.used);
    if (n == 0) {
        std::cout << "Client closed fd=" << fd_ << std::endl;
        pool_.release(read_buf_);
        pool_.release(write_buf_);
        reactor_.schedule_close(fd_);
        return;
    } else if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        perror("read error");   
        reactor_.schedule_close(fd_);
        return;
    }
    read_buf_.used += n;

    std::memcpy(write_buf_.data, read_buf_.data, read_buf_.used);
    write_size_ = read_buf_.used;
    write_offset_ = 0;

    handle_backpressure();

    on_writeable();
}

void Connection::on_writeable() {
    if (write_size_ == 0) return;
    ssize_t n = write(fd_, write_buf_.data + write_offset_, write_size_ - write_offset_);
    if (n > 0) write_offset_ += n;

    if (write_offset_ == write_size_) {
        write_size_ = 0;
        write_offset_ = 0;
        read_buf_.used = 0;

        reactor_.enable_read(fd_);
    }
}

int Connection::get_fd() {
    return fd_;
}

void Connection::handle_backpressure() {
    if (write_size_ >= write_buf_.size) {
        reactor_.disable_read(fd_);
    } else {
        reactor_.enable_read(fd_);
    }
}
