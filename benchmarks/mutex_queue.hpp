#ifndef URINGCORE_BENCHMARKS_MUTEX_QUEUE_HPP
#define URINGCORE_BENCHMARKS_MUTEX_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace uringcore::benchmarks {

using Task = std::function<void()>;

class MutexTaskQueue {
public:
    void push(Task task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(task));
        }
        cv_.notify_one();
    }

    bool pop(Task& task) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        task = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<Task> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace uringcore::benchmarks

#endif // URINGCORE_BENCHMARKS_MUTEX_QUEUE_HPP
