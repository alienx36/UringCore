#include "net/connection.hpp"
#include <unistd.h>
#include <cstring>

namespace uringcore::net {

Connection::~Connection() {
    reset();
}

void Connection::init(int fd) {
    fd_ = fd;
    state_ = ConnectionState::READING;
    read_offset_ = 0;
    bytes_read_ = 0;
    bytes_written_ = 0;
    
    std::memset(buffer_, 0, BUFFER_SIZE);
    
    io_event_.fd = fd_;
    io_event_.type = uringcore::core::EventType::READ;
    io_event_.user_data = this;
    io_event_.res = 0;
}

void Connection::reset() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    state_ = ConnectionState::DISCONNECTED;
    bytes_read_ = 0;
    bytes_written_ = 0;
    io_event_.user_data = nullptr;
}

} // namespace uringcore::net
