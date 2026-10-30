#ifndef URINGCORE_NET_BUFFER_RING_HPP
#define URINGCORE_NET_BUFFER_RING_HPP

#include <liburing.h>
#include <cstdint>
#include <cstddef>

namespace uringcore::net {

constexpr std::size_t RING_BUFFER_SIZE = 4096;
constexpr uint16_t BG_ID = 1; // Buffer Group ID

class BufferRing {
public:
    BufferRing() = default;
    ~BufferRing();

    bool init(struct io_uring* ring, uint32_t ring_entries);
    void cleanup();

    uint8_t* get_buffer(uint16_t buf_id) {
        return raw_buf_pool_ + (buf_id * RING_BUFFER_SIZE);
    }

    void recycle_buffer(uint16_t buf_id);

private:
    struct io_uring* ring_{nullptr};
    struct io_uring_buf_ring* buf_ring_{nullptr};
    uint8_t* raw_buf_pool_{nullptr};
    uint32_t ring_entries_{0};
    uint16_t bgid_{BG_ID};
};

} // namespace uringcore::net

#endif // URINGCORE_NET_BUFFER_RING_HPP
