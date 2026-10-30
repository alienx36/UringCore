#include "core/uring_engine.hpp"
#include <iostream>

namespace uringcore::core {

UringEngine::UringEngine() = default;

UringEngine::~UringEngine() {
    shutdown();
}

bool UringEngine::init(uint32_t queue_depth) {
    if (initialized_) return true;

    int ret = io_uring_queue_init(queue_depth, &ring_, 0);
    if (ret < 0) {
        std::cerr << "[UringEngine] Failed to initialize io_uring: " << ret << "\n";
        return false;
    }
    initialized_ = true;
    return true;
}

bool UringEngine::submit_accept(int server_fd, IOEvent* event) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    if (!sqe) return false;

    io_uring_prep_accept(sqe, server_fd, nullptr, nullptr, 0);
    io_uring_sqe_set_data(sqe, event);
    return io_uring_submit(&ring_) >= 0;
}

bool UringEngine::submit_read(int fd, void* buffer, std::size_t len, IOEvent* event) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    if (!sqe) return false;

    io_uring_prep_read(sqe, fd, buffer, len, 0);
    io_uring_sqe_set_data(sqe, event);
    return io_uring_submit(&ring_) >= 0;
}

bool UringEngine::submit_write(int fd, const void* buffer, std::size_t len, IOEvent* event) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    if (!sqe) return false;

    io_uring_prep_write(sqe, fd, buffer, len, 0);
    io_uring_sqe_set_data(sqe, event);
    return io_uring_submit(&ring_) >= 0;
}

bool UringEngine::submit_cancel(IOEvent* target_event, IOEvent* cancel_event) {
    if (!initialized_ || !target_event) return false;

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
    if (!sqe) return false;

    io_uring_prep_cancel64(sqe, reinterpret_cast<uint64_t>(target_event), 0);
    io_uring_sqe_set_data(sqe, cancel_event);
    return io_uring_submit(&ring_) >= 0;
}

int UringEngine::process_events(uint32_t max_events, int timeout_ms) {
    if (!initialized_) return -1;

    struct io_uring_cqe* cqe{nullptr};

    if (timeout_ms > 0) {
        struct __kernel_timespec ts{};
        ts.tv_sec = timeout_ms / 1000;
        ts.tv_nsec = (timeout_ms % 1000) * 1000000LL;
        int ret = io_uring_wait_cqe_timeout(&ring_, &cqe, &ts);
        if (ret < 0) return 0;
    } else if (timeout_ms == 0) {
        int ret = io_uring_peek_cqe(&ring_, &cqe);
        if (ret < 0 || !cqe) return 0;
    } else {
        int ret = io_uring_wait_cqe(&ring_, &cqe);
        if (ret < 0) return 0;
    }

    unsigned head;
    int processed = 0;

    io_uring_for_each_cqe(&ring_, head, cqe) {
        auto* event = static_cast<IOEvent*>(io_uring_cqe_get_data(cqe));
        if (event) {
            event->res = cqe->res;
        }
        processed++;
        if (static_cast<uint32_t>(processed) >= max_events) break;
    }
    io_uring_cq_advance(&ring_, processed);

    return processed;
}

void UringEngine::shutdown() {
    if (initialized_) {
        io_uring_queue_exit(&ring_);
        initialized_ = false;
    }
}

} // namespace uringcore::core
