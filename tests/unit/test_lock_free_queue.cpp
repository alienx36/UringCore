#include "concurrency/lock_free_queue.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>

using namespace uringcore::concurrency;

void test_single_threaded() {
    LockFreeQueue<int> q;
    q.enqueue(10);
    q.enqueue(20);

    auto item1 = q.dequeue();
    assert(item1.has_value() && *item1 == 10);

    auto item2 = q.dequeue();
    assert(item2.has_value() && *item2 == 20);

    auto empty_item = q.dequeue();
    assert(!empty_item.has_value());

    std::cout << "[PASS] LockFreeQueue single-threaded test\n";
}

void test_multi_threaded() {
    LockFreeQueue<int> q;
    constexpr int num_producers = 4;
    constexpr int items_per_producer = 1000;
    std::vector<std::thread> producers;

    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&q, i]() {
            for (int j = 0; j < items_per_producer; ++j) {
                q.enqueue(i * items_per_producer + j);
            }
        });
    }

    for (auto& t : producers) {
        t.join();
    }

    int count = 0;
    while (q.dequeue().has_value()) {
        count++;
    }

    assert(count == num_producers * items_per_producer);
    std::cout << "[PASS] LockFreeQueue multi-threaded test (" << count << " items processed)\n";
}

int main() {
    test_single_threaded();
    test_multi_threaded();
    return 0;
}
