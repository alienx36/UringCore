#ifndef URINGCORE_CORE_WORKER_GROUP_HPP
#define URINGCORE_CORE_WORKER_GROUP_HPP

#include "core/worker.hpp"
#include <vector>
#include <memory>

namespace uringcore::core {

class WorkerGroup {
public:
    explicit WorkerGroup(uint32_t num_workers);
    ~WorkerGroup();

    bool listen_all(uint16_t port);
    bool start_all(uint32_t queue_depth = 1024);
    bool listen_and_start(uint16_t port, uint32_t queue_depth = 1024);
    void stop();

    bool post_task(const Task& task);
    bool post_task_to(uint32_t worker_idx, const Task& task);

    size_t size() const noexcept { return workers_.size(); }
    Worker* get_worker(size_t index) { return index < workers_.size() ? workers_[index].get() : nullptr; }

private:
    std::vector<std::unique_ptr<Worker>> workers_;
    std::atomic<uint32_t> next_worker_{0};
};

} // namespace uringcore::core

#endif // URINGCORE_CORE_WORKER_GROUP_HPP
