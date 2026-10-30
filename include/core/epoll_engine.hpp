#ifndef URINGCORE_CORE_EPOLL_ENGINE_HPP
#define URINGCORE_CORE_EPOLL_ENGINE_HPP

#include "core/engine_interface.hpp"
#include <sys/epoll.h>
#include <vector>

namespace uringcore::core {

class EpollEngine : public IEngine {
public:
    EpollEngine();
    ~EpollEngine() override;

    bool init(uint32_t queue_depth) override;
    bool submit_accept(int server_fd, IOEvent* event) override;
    bool submit_read(int fd, void* buffer, std::size_t len, IOEvent* event) override;
    bool submit_write(int fd, const void* buffer, std::size_t len, IOEvent* event) override;
    bool submit_cancel(IOEvent* target_event, IOEvent* cancel_event) override;
    int process_events(uint32_t max_events, int timeout_ms) override;
    void shutdown() override;

private:
    int epoll_fd_{-1};
    std::vector<struct epoll_event> events_;
};

} // namespace uringcore::core

#endif // URINGCORE_CORE_EPOLL_ENGINE_HPP
