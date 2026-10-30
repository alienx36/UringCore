#ifndef URINGCORE_CONCURRENCY_CPU_AFFINITY_HPP
#define URINGCORE_CONCURRENCY_CPU_AFFINITY_HPP

#include <pthread.h>
#include <cstdint>

namespace uringcore::concurrency {

class CpuAffinity {
public:
    // Pins the specified thread (default: current thread) to a specific CPU core ID
    static bool pin_thread_to_core(uint32_t core_id, pthread_t thread = ::pthread_self());

    // Queries total available online CPU cores on the system
    static uint32_t get_core_count();
};

} // namespace uringcore::concurrency

#endif // URINGCORE_CONCURRENCY_CPU_AFFINITY_HPP
