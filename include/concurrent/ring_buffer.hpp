#ifndef URINGCORE_CONCURRENT_RING_BUFFER_HPP
#define URINGCORE_CONCURRENT_RING_BUFFER_HPP

#include <atomic>
#include <vector>
#include <cstddef>
#include <utility>

namespace uringcore::concurrent {

// Single-Producer Single-Consumer (SPSC) Lock-Free Bounded Ring Buffer
template <typename T, std::size_t Capacity = 1024>
class RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

public:
    RingBuffer() : buffer_(Capacity) {}

    // Push item by copy
    bool push(const T& item) {
        return emplace(item);
    }

    // Push item by move
    bool push(T&& item) {
        return emplace(std::move(item));
    }

    // Perfect forwarding push
    template <typename... Args>
    bool emplace(Args&&... args) {
        const auto current_tail = tail_.load(std::memory_order_relaxed);
        const auto current_head = head_.load(std::memory_order_acquire);

        if (current_tail - current_head >= Capacity) {
            return false; // Ring buffer is full
        }

        buffer_[current_tail & mask_] = T(std::forward<Args>(args)...);
        tail_.store(current_tail + 1, std::memory_order_release);
        return true;
    }

    // Pop item from buffer (Consumer thread only)
    bool pop(T& value) {
        const auto current_head = head_.load(std::memory_order_relaxed);
        const auto current_tail = tail_.load(std::memory_order_acquire);

        if (current_head == current_tail) {
            return false; // Ring buffer is empty
        }

        value = std::move(buffer_[current_head & mask_]);
        head_.store(current_head + 1, std::memory_order_release);
        return true;
    }

    bool empty() const noexcept {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

    std::size_t size() const noexcept {
        const auto current_head = head_.load(std::memory_order_relaxed);
        const auto current_tail = tail_.load(std::memory_order_relaxed);
        return (current_tail >= current_head) ? (current_tail - current_head) : 0;
    }

private:
    static constexpr std::size_t mask_ = Capacity - 1;
    std::vector<T> buffer_;

    // Hardcoded 64-byte alignment to prevent false sharing and compiler interference warnings
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
};

} // namespace uringcore::concurrent

#endif // URINGCORE_CONCURRENT_RING_BUFFER_HPP
