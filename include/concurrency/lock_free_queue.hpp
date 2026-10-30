#ifndef URINGCORE_CONCURRENCY_LOCK_FREE_QUEUE_HPP
#define URINGCORE_CONCURRENCY_LOCK_FREE_QUEUE_HPP

#include <atomic>
#include <memory>
#include <optional>

namespace uringcore::concurrency {

template <typename T>
class LockFreeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next{nullptr};
        Node() = default;
        explicit Node(std::shared_ptr<T> val) : data(std::move(val)) {}
    };

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;

public:
    LockFreeQueue() {
        Node* dummy = new Node();
        head_.store(dummy, std::memory_order_relaxed);
        tail_.store(dummy, std::memory_order_relaxed);
    }

    ~LockFreeQueue() {
        while (Node* old_head = head_.load(std::memory_order_relaxed)) {
            head_.store(old_head->next.load(std::memory_order_relaxed), std::memory_order_relaxed);
            delete old_head;
        }
    }

    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    void enqueue(T value) {
        auto new_data = std::make_shared<T>(std::move(value));
        Node* new_node = new Node(new_data);
        
        Node* prev_tail = tail_.exchange(new_node, std::memory_order_acq_rel);
        prev_tail->next.store(new_node, std::memory_order_release);
    }

    std::optional<T> dequeue() {
        Node* old_head = head_.load(std::memory_order_acquire);
        Node* next = old_head->next.load(std::memory_order_acquire);

        if (!next) {
            return std::nullopt; // Queue empty
        }

        std::shared_ptr<T> result = next->data;
        head_.store(next, std::memory_order_release);
        delete old_head;

        if (result) {
            return *result;
        }
        return std::nullopt;
    }

    bool empty() const {
        Node* head = head_.load(std::memory_order_relaxed);
        Node* next = head->next.load(std::memory_order_relaxed);
        return next == nullptr;
    }
};

} // namespace uringcore::concurrency

#endif // URINGCORE_CONCURRENCY_LOCK_FREE_QUEUE_HPP
