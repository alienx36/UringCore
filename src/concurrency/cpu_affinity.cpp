#include "concurrency/cpu_affinity.hpp"
#include <sched.h>
#include <unistd.h>
#include <iostream>

namespace uringcore::concurrency {

bool CpuAffinity::pin_thread_to_core(uint32_t core_id, pthread_t thread) {
    uint32_t num_cores = get_core_count();
    if (core_id >= num_cores) {
        std::cerr << "[WARNING] Core ID " << core_id << " exceeds available online cores (" << num_cores << ")\n";
        return false;
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    int rc = ::pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::cerr << "[ERROR] pthread_setaffinity_np failed for core " << core_id << " with rc: " << rc << "\n";
        return false;
    }

    return true;
}

uint32_t CpuAffinity::get_core_count() {
    long count = ::sysconf(_SC_NPROCESSORS_ONLN);
    return count > 0 ? static_cast<uint32_t>(count) : 1;
}

} // namespace uringcore::concurrency
