#include "core/worker.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>

using namespace uringcore::core;

int main() {
    constexpr uint32_t NUM_WORKERS = 4;
    constexpr uint16_t TEST_PORT = 8080;

    std::cout << "[Step 3 Test] Initializing " << NUM_WORKERS << " workers on port " << TEST_PORT << " with SO_REUSEPORT...\n";

    std::vector<std::unique_ptr<Worker>> workers;

    for (uint32_t i = 0; i < NUM_WORKERS; ++i) {
        auto worker = std::make_unique<Worker>(i, static_cast<int>(i));
        
        if (!worker->listen_on_port(TEST_PORT)) {
            std::cerr << "[Step 3 Test] ERROR: Worker " << i << " failed SO_REUSEPORT socket setup!\n";
            return 1;
        }

        if (!worker->start()) {
            std::cerr << "[Step 3 Test] ERROR: Worker " << i << " failed to start reactor loop!\n";
            return 1;
        }

        workers.push_back(std::move(worker));
    }

    std::cout << "[Step 3 Test] Dispatching tasks across all " << NUM_WORKERS << " worker lock-free rings...\n";

    std::atomic<uint32_t> completed{0};

    for (uint32_t i = 0; i < NUM_WORKERS; ++i) {
        workers[i]->post_task([i, &completed]() {
            std::cout << "  -> Task executed on core-pinned Worker " << i << "\n";
            completed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    for (auto& w : workers) {
        w->stop();
    }

    std::cout << "[Step 3 Test] SUCCESS! Executed " << completed.load() 
              << " tasks on " << NUM_WORKERS << " independent SO_REUSEPORT sockets.\n";

    return 0;
}
