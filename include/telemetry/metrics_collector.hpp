#ifndef URINGCORE_TELEMETRY_METRICS_COLLECTOR_HPP
#define URINGCORE_TELEMETRY_METRICS_COLLECTOR_HPP

#include <atomic>
#include <cstdint>

namespace uringcore::telemetry {

struct alignas(64) WorkerMetrics {
    std::atomic<uint64_t> operations_processed{0};
    std::atomic<uint64_t> bytes_transferred{0};
    std::atomic<uint64_t> active_connections{0};
    char padding[40]; // Complete 64-byte cache line
};

class MetricsCollector {
public:
    static MetricsCollector& instance() {
        static MetricsCollector instance;
        return instance;
    }

    WorkerMetrics worker_metrics[64]; // Supports up to 64 pinned worker threads

    void record_op(size_t worker_id, uint64_t bytes) {
        if (worker_id < 64) {
            worker_metrics[worker_id].operations_processed.fetch_add(1, std::memory_order_relaxed);
            worker_metrics[worker_id].bytes_transferred.fetch_add(bytes, std::memory_order_relaxed);
        }
    }
};

} // namespace uringcore::telemetry

#endif // URINGCORE_TELEMETRY_METRICS_COLLECTOR_HPP
