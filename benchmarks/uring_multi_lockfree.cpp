#include "core/worker_group.hpp"
#include "net/socket.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

using namespace uringcore;

static std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running.store(false, std::memory_order_relaxed);
}

int main() {
    constexpr uint32_t NUM_WORKERS = 4;
    constexpr uint16_t PORT = 8083;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    core::WorkerGroup group(NUM_WORKERS);

    if (!group.listen_all(PORT)) {
        std::cerr << "[FATAL] Lock-free engine failed to bind to port " << PORT << "\n";
        return 1;
    }

    if (!group.start_all(2048)) {
        std::cerr << "[FATAL] Lock-free engine failed to start worker loops\n";
        return 1;
    }

    std::cout << "[Uring Multi-Lockfree] 4 Cores Pinned + Lock-Free Atomics listening on port " << PORT << "...\n";

    while (g_running.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    group.stop();
    return 0;
}
