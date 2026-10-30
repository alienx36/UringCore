#include "net/socket.hpp"
#include "core/worker_group.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace uringcore;

int main() {
    constexpr uint32_t NUM_WORKERS = 4;
    constexpr uint16_t PORT = 8080;

    core::WorkerGroup group(NUM_WORKERS);

    if (!group.listen_all(PORT)) {
        std::cerr << "Failed to bind workers to port " << PORT << "\n";
        return 1;
    }

    if (!group.start_all(2048)) {
        std::cerr << "Failed to start worker event loops\n";
        return 1;
    }

    std::cout << "[Uring Throughput] Multi-worker engine running on port " << PORT 
              << " (" << NUM_WORKERS << " cores with SO_REUSEPORT)...\n";

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    group.stop();
    return 0;
}
