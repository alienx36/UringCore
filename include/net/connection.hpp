#ifndef URINGCORE_NET_CONNECTION_HPP
#define URINGCORE_NET_CONNECTION_HPP

#include "core/engine_interface.hpp"
#include <cstdint>
#include <cstddef>

namespace uringcore::net {

constexpr std::size_t BUFFER_SIZE = 4096;

enum class ConnectionState {
    DISCONNECTED,
    READING,
    WRITING,
    CLOSING
};

class Connection {
public:
    Connection() = default;
    ~Connection();

    void init(int fd);
    void reset();

    int get_fd() const { return fd_; }
    ConnectionState get_state() const { return state_; }
    void set_state(ConnectionState state) { state_ = state; }

    uint8_t* get_read_buffer() { return buffer_; }
    uint8_t* get_write_buffer() { return buffer_ + read_offset_; }
    
    std::size_t get_read_bytes() const { return bytes_read_; }
    void set_read_bytes(std::size_t bytes) { bytes_read_ = bytes; }

    std::size_t get_written_bytes() const { return bytes_written_; }
    void add_written_bytes(std::size_t bytes) { bytes_written_ += bytes; }

    uringcore::core::IOEvent* get_io_event() { return &io_event_; }

    // Offset arithmetic for partial network writes
    std::size_t get_remaining_write_bytes() const {
        return bytes_read_ > bytes_written_ ? bytes_read_ - bytes_written_ : 0;
    }
    
    const uint8_t* get_pending_write_ptr() const {
        return buffer_ + bytes_written_;
    }

private:
    int fd_{-1};
    ConnectionState state_{ConnectionState::DISCONNECTED};
    
    uint8_t buffer_[BUFFER_SIZE]{};
    std::size_t read_offset_{0};
    std::size_t bytes_read_{0};
    std::size_t bytes_written_{0};

    uringcore::core::IOEvent io_event_{};
};

} // namespace uringcore::net

#endif // URINGCORE_NET_CONNECTION_HPP
