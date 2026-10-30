#ifndef URINGCORE_CORE_URING_ENGINE_HPP
#define URINGCORE_CORE_URING_ENGINE_HPP

#include "core/engine_interface.hpp"
#include <liburing.h>

namespace uringcore::core {

class UringEngine : public IEngine {
public:
    UringEngine();
    ~UringEngine() override;

    bool init(uint32_t queue_depth) override;
    bool submit_accept(int server_fd, IOEvent* event) override;
    bool submit_read(int fd, void* buffer, std::size_t len, IOEvent* event) override;
    bool submit_write(int fd, const void* buffer, std::size_t len, IOEvent* event) override;
    bool submit_cancel(IOEvent* target_event, IOEvent* cancel_event) override;
    int process_events(uint32_t max_events, int timeout_ms) override;
    void shutdown() override;

    struct io_uring* get_ring() { return &ring_; }

private:
    struct io_uring ring_{};
    bool initialized_{false};
};

} // namespace uringcore::core

#endif // URINGCORE_CORE_URING_ENGINE_HPP
