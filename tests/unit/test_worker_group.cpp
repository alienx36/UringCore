#include "core/worker_group.hpp"
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>

using namespace uringcore::core;

int main() {
    constexpr uint32_t NUM_WORKERS = 4;
    constexpr uint16_t TEST_PORT = 8081;

    std::cout << "[Step 4 Test] Initializing WorkerGroup with " << NUM_WORKERS << " workers...\n";

    WorkerGroup group(NUM_WORKERS);

    if (!group.listen_and_start(TEST_PORT)) {
        std::cerr << "[Step 4 Test] ERROR: Failed to start WorkerGroup on port " << TEST_PORT << "\n";
        return 1;
    }

    constexpr uint32_t TOTAL_TASKS = 100;
    std::atomic<uint32_t> completed{0};

    // 1. Test targeted task routing
    std::cout << "[Step 4 Test] Posting targeted tasks directly to specific worker IDs...\n";
    for (uint32_t i = 0; i < NUM_WORKERS; ++i) {
        group.post_task_to(i, [i, &completed]() {
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    // 2. Test round-robin task dispatch
    std::cout << "[Step 4 Test] Dispatching " << TOTAL_TASKS << " round-robin tasks across WorkerGroup...\n";
    for (uint32_t i = 0; i < TOTAL_TASKS; ++i) {
        group.post_task([&completed]() {
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    group.stop();

    uint32_t expected = NUM_WORKERS + TOTAL_TASKS;
    uint32_t actual = completed.load();

    if (actual == expected) {
        std::cout << "[Step 4 Test] SUCCESS! Executed all " << actual << " tasks cleanly across WorkerGroup.\n";
        return 0;
    } else {
        std::cerr << "[Step 4 Test] ERROR: Expected " << expected << " tasks, but got " << actual << "\n";
        return 1;
    }
}
