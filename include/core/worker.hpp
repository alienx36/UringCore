#ifndef URINGCORE_CORE_WORKER_HPP
#define URINGCORE_CORE_WORKER_HPP

#include "core/uring_engine.hpp"
#include "net/socket.hpp"
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>

namespace uringcore::core {

using Task = std::function<void()>;

class Worker {
public:
    explicit Worker(uint32_t worker_id, int cpu_core_id);
    ~Worker();

    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;

    bool listen_on_port(uint16_t port);
    bool start(uint32_t queue_depth = 1024);
    void stop();

    bool post_task(const Task& task);
    bool post_task(Task&& task);

    uint32_t id() const noexcept { return worker_id_; }
    int cpu_core() const noexcept { return cpu_core_id_; }
    int listen_fd() const noexcept { return listen_fd_; }
    bool is_running() const noexcept { return running_.load(std::memory_order_relaxed); }

private:
    void run();

    uint32_t worker_id_;
    int cpu_core_id_;
    int listen_fd_{-1};
    std::atomic<bool> running_{false};
    UringEngine engine_;
    std::thread thread_;

    std::mutex task_mutex_;
    std::queue<Task> task_queue_;
};

} // namespace uringcore::core

#endif // URINGCORE_CORE_WORKER_HPP
