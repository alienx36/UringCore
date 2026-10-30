#include "core/worker_group.hpp"

namespace uringcore::core {

WorkerGroup::WorkerGroup(uint32_t num_workers) {
    for (uint32_t i = 0; i < num_workers; ++i) {
        workers_.push_back(std::make_unique<Worker>(i, i));
    }
}

WorkerGroup::~WorkerGroup() {
    stop();
}

bool WorkerGroup::listen_all(uint16_t port) {
    for (auto& worker : workers_) {
        if (!worker->listen_on_port(port)) return false;
    }
    return true;
}

bool WorkerGroup::start_all(uint32_t queue_depth) {
    for (auto& worker : workers_) {
        if (!worker->start(queue_depth)) return false;
    }
    return true;
}

bool WorkerGroup::listen_and_start(uint16_t port, uint32_t queue_depth) {
    return listen_all(port) && start_all(queue_depth);
}

void WorkerGroup::stop() {
    for (auto& worker : workers_) {
        worker->stop();
    }
}

bool WorkerGroup::post_task(const Task& task) {
    if (workers_.empty()) return false;
    uint32_t idx = next_worker_.fetch_add(1, std::memory_order_relaxed) % workers_.size();
    return workers_[idx]->post_task(task);
}

bool WorkerGroup::post_task_to(uint32_t worker_idx, const Task& task) {
    if (worker_idx >= workers_.size()) return false;
    return workers_[worker_idx]->post_task(task);
}

} // namespace uringcore::core
