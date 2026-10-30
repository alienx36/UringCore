#include "core/epoll_engine.hpp"
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <cerrno>

namespace uringcore::core {

EpollEngine::EpollEngine() = default;

EpollEngine::~EpollEngine() {
    shutdown();
}

bool EpollEngine::init(uint32_t queue_depth) {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        std::cerr << "[EpollEngine] Failed to create epoll instance.\n";
        return false;
    }
    events_.resize(queue_depth);
    return true;
}

bool EpollEngine::submit_accept(int server_fd, IOEvent* event) {
    struct epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = event;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd, &ev) == 0;
}

bool EpollEngine::submit_read(int fd, void* /*buffer*/, std::size_t /*len*/, IOEvent* event) {
    struct epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = event;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0) {
        return true;
    }
    // Fallback: If FD was not added yet (ENOENT), perform EPOLL_CTL_ADD
    if (errno == ENOENT) {
        return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
    }
    return false;
}

bool EpollEngine::submit_write(int fd, const void* /*buffer*/, std::size_t /*len*/, IOEvent* event) {
    struct epoll_event ev{};
    ev.events = EPOLLOUT | EPOLLET;
    ev.data.ptr = event;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0) {
        return true;
    }
    if (errno == ENOENT) {
        return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
    }
    return false;
}

bool EpollEngine::submit_cancel(IOEvent* target_event, IOEvent* cancel_event) {
    if (!target_event || epoll_fd_ < 0) return false;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, target_event->fd, nullptr);
    if (cancel_event) {
        cancel_event->res = (ret == 0) ? 0 : -1;
    }
    return ret == 0;
}

int EpollEngine::process_events(uint32_t max_events, int timeout_ms) {
    if (epoll_fd_ < 0) return -1;

    uint32_t count = std::min(max_events, static_cast<uint32_t>(events_.size()));
    int nfds = epoll_wait(epoll_fd_, events_.data(), count, timeout_ms);

    for (int i = 0; i < nfds; ++i) {
        auto* event = static_cast<IOEvent*>(events_[i].data.ptr);
        if (event) {
            // Execution hook
        }
    }
    return nfds;
}

void EpollEngine::shutdown() {
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

} // namespace uringcore::core
