#include "net/buffer_ring.hpp"
#include <iostream>
#include <cstdlib>

namespace uringcore::net {

BufferRing::~BufferRing() {
    cleanup();
}

bool BufferRing::init(struct io_uring* ring, uint32_t ring_entries) {
    ring_ = ring;
    ring_entries_ = ring_entries;

    // 1. Allocate page-aligned buffer pool for packet payloads
    std::size_t pool_size = ring_entries_ * RING_BUFFER_SIZE;
    void* pool_ptr = nullptr;
    if (posix_memalign(&pool_ptr, 4096, pool_size) != 0) {
        std::cerr << "[BufferRing] Failed to allocate page-aligned buffer pool.\n";
        return false;
    }
    raw_buf_pool_ = static_cast<uint8_t*>(pool_ptr);

    // 2. Allocate page-aligned memory for the ring descriptors array
    std::size_t ring_size = sizeof(struct io_uring_buf_ring) + 
                            (ring_entries_ * sizeof(struct io_uring_buf));
    void* ring_ptr = nullptr;
    if (posix_memalign(&ring_ptr, 4096, ring_size) != 0) {
        std::cerr << "[BufferRing] Failed to allocate page-aligned ring memory.\n";
        std::free(raw_buf_pool_);
        raw_buf_pool_ = nullptr;
        return false;
    }
    buf_ring_ = static_cast<struct io_uring_buf_ring*>(ring_ptr);

    // 3. Register user-allocated ring buffer with kernel
    struct io_uring_buf_reg reg{};
    reg.ring_addr = reinterpret_cast<uint64_t>(buf_ring_);
    reg.ring_entries = ring_entries_;
    reg.bgid = bgid_;
    reg.flags = 0;

    int ret = io_uring_register_buf_ring(ring_, &reg, 0);
    if (ret < 0) {
        std::cerr << "[BufferRing] Failed to register buffer ring: " << ret << "\n";
        std::free(buf_ring_);
        std::free(raw_buf_pool_);
        buf_ring_ = nullptr;
        raw_buf_pool_ = nullptr;
        return false;
    }

    io_uring_buf_ring_init(buf_ring_);

    // 4. Populate buffer ring entries
    for (uint32_t i = 0; i < ring_entries_; ++i) {
        io_uring_buf_ring_add(
            buf_ring_,
            raw_buf_pool_ + (i * RING_BUFFER_SIZE),
            RING_BUFFER_SIZE,
            static_cast<uint16_t>(i),
            io_uring_buf_ring_mask(ring_entries_),
            i
        );
    }

    io_uring_buf_ring_advance(buf_ring_, ring_entries_);
    return true;
}

void BufferRing::recycle_buffer(uint16_t buf_id) {
    if (!buf_ring_ || !raw_buf_pool_) return;

    io_uring_buf_ring_add(
        buf_ring_,
        raw_buf_pool_ + (buf_id * RING_BUFFER_SIZE),
        RING_BUFFER_SIZE,
        buf_id,
        io_uring_buf_ring_mask(ring_entries_),
        0
    );
    io_uring_buf_ring_advance(buf_ring_, 1);
}

void BufferRing::cleanup() {
    if (buf_ring_ && ring_) {
        io_uring_unregister_buf_ring(ring_, bgid_);
        std::free(buf_ring_);
        buf_ring_ = nullptr;
    }
    if (raw_buf_pool_) {
        std::free(raw_buf_pool_);
        raw_buf_pool_ = nullptr;
    }
}

} // namespace uringcore::net
