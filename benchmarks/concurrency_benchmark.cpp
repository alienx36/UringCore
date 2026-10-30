#include "core/worker_group.hpp"
#include "mutex_queue.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <iomanip>

using namespace uringcore::core;
using namespace uringcore::benchmarks;

constexpr uint32_t TOTAL_OPERATIONS = 1'000'000;
constexpr uint32_t NUM_PRODUCERS = 4;
constexpr uint32_t NUM_WORKERS = 4;

void run_mutex_benchmark() {
    MutexTaskQueue queue;
    std::atomic<uint32_t> completed{0};
    std::atomic<bool> running{true};

    // Spawn Mutex Consumer Threads
    std::vector<std::thread> consumers;
    for (uint32_t i = 0; i < NUM_WORKERS; ++i) {
        consumers.emplace_back([&queue, &completed, &running]() {
            while (running.load(std::memory_order_relaxed) || !queue.empty()) {
                Task task;
                if (queue.pop(task)) {
                    if (task) task();
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Spawn Producer Threads
    std::vector<std::thread> producers;
    uint32_t ops_per_producer = TOTAL_OPERATIONS / NUM_PRODUCERS;

    for (uint32_t p = 0; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&queue, &completed, ops_per_producer]() {
            for (uint32_t i = 0; i < ops_per_producer; ++i) {
                queue.push([&completed]() {
                    completed.fetch_add(1, std::memory_order_relaxed);
                });
            }
        });
    }

    for (auto& t : producers) t.join();

    // Wait for all tasks to complete
    while (completed.load(std::memory_order_relaxed) < TOTAL_OPERATIONS) {
        std::this_thread::yield();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    running.store(false);
    for (auto& t : consumers) t.join();

    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
    double ops_per_sec = (TOTAL_OPERATIONS / elapsed.count()) * 1000.0;

    std::cout << "[Mutex Engine]     Ops: " << TOTAL_OPERATIONS 
              << " | Time: " << std::fixed << std::setprecision(2) << elapsed.count() << " ms"
              << " | Throughput: " << static_cast<uint64_t>(ops_per_sec) << " ops/sec\n";
}

void run_lockfree_benchmark() {
    WorkerGroup group(NUM_WORKERS);
    // Boot workers without socket listen overhead for pure concurrency testing
    for (uint32_t i = 0; i < group.size(); ++i) {
        group.get_worker(i)->start(1024);
    }

    std::atomic<uint32_t> completed{0};
    auto start_time = std::chrono::high_resolution_clock::now();

    // Spawn Producer Threads (Each producer targets workers safely to honor SPSC guarantees)
    std::vector<std::thread> producers;
    uint32_t ops_per_producer = TOTAL_OPERATIONS / NUM_PRODUCERS;

    for (uint32_t p = 0; p < NUM_PRODUCERS; ++p) {
        producers.emplace_back([&group, &completed, ops_per_producer, p]() {
            uint32_t target_worker = p % NUM_WORKERS;
            for (uint32_t i = 0; i < ops_per_producer; ++i) {
                while (!group.post_task_to(target_worker, [&completed]() {
                    completed.fetch_add(1, std::memory_order_relaxed);
                })) {
                    std::this_thread::yield(); // Backoff if target SPSC ring is full
                }
            }
        });
    }

    for (auto& t : producers) t.join();

    // Wait for all tasks to complete
    while (completed.load(std::memory_order_relaxed) < TOTAL_OPERATIONS) {
        std::this_thread::yield();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    group.stop();

    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
    double ops_per_sec = (TOTAL_OPERATIONS / elapsed.count()) * 1000.0;

    std::cout << "[Lock-Free Engine] Ops: " << TOTAL_OPERATIONS 
              << " | Time: " << std::fixed << std::setprecision(2) << elapsed.count() << " ms"
              << " | Throughput: " << static_cast<uint64_t>(ops_per_sec) << " ops/sec\n";
}

int main() {
    std::cout << "===============================================================\n";
    std::cout << "  UringCore Phase 3, Step 5: Concurrency Performance Benchmark  \n";
    std::cout << "  Operations: " << TOTAL_OPERATIONS << " | Workers: " << NUM_WORKERS << " | Producers: " << NUM_PRODUCERS << "\n";
    std::cout << "===============================================================\n\n";

    run_mutex_benchmark();
    run_lockfree_benchmark();

    std::cout << "\n===============================================================\n";
    return 0;
}
