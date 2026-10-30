#ifndef URINGCORE_CONCURRENCY_THREAD_POOL_HPP
#define URINGCORE_CONCURRENCY_THREAD_POOL_HPP

#include "core/worker_group.hpp"

namespace uringcore::concurrency {

// Canonical alias mapping WorkerGroup to thread_pool.hpp spec path
using ThreadPool = uringcore::core::WorkerGroup;

} // namespace uringcore::concurrency

#endif // URINGCORE_CONCURRENCY_THREAD_POOL_HPP

