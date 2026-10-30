#ifndef URINGCORE_CORE_ENGINE_INTERFACE_HPP
#define URINGCORE_CORE_ENGINE_INTERFACE_HPP

#include <cstdint>
#include <cstddef>

namespace uringcore::core {

enum class EventType {
    ACCEPT,
    READ,
    WRITE
};

struct IOEvent {
    int fd{-1};
    EventType type{EventType::READ};
    int32_t res{0};
    void* user_data{nullptr};
};

class IEngine {
public:
    virtual ~IEngine() = default;

    virtual bool init(uint32_t queue_depth) = 0;
    virtual bool submit_accept(int server_fd, IOEvent* event) = 0;
    virtual bool submit_read(int fd, void* buffer, std::size_t len, IOEvent* event) = 0;
    virtual bool submit_write(int fd, const void* buffer, std::size_t len, IOEvent* event) = 0;
    virtual bool submit_cancel(IOEvent* target_event, IOEvent* cancel_event) = 0;
    virtual int process_events(uint32_t max_events, int timeout_ms) = 0;
    virtual void shutdown() = 0;
};

} // namespace uringcore::core

#endif // URINGCORE_CORE_ENGINE_INTERFACE_HPP
