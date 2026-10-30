cmake_minimum_required(VERSION 3.16)
project(UringCore CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -pthread")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core Static Library
add_library(uringcore_core STATIC
    src/core/epoll_engine.cpp
    src/core/uring_engine.cpp
    src/core/worker.cpp
    src/core/worker_group.cpp
    src/concurrency/cpu_affinity.cpp
    src/net/buffer_ring.cpp
    src/net/connection.cpp
    src/net/socket.cpp
)

target_link_libraries(uringcore_core PUBLIC uring pthread)

# Benchmark Executables (5-Variant Matrix)
add_executable(posix_baseline benchmarks/posix_baseline.cpp)
target_link_libraries(posix_baseline PRIVATE uringcore_core)

add_executable(uring_single_thread benchmarks/uring_single_thread.cpp)
target_link_libraries(uring_single_thread PRIVATE uringcore_core)

add_executable(uring_throughput benchmarks/uring_throughput.cpp)
target_link_libraries(uring_throughput PRIVATE uringcore_core)

add_executable(concurrency_benchmark benchmarks/concurrency_benchmark.cpp)
target_link_libraries(concurrency_benchmark PRIVATE uringcore_core)

add_executable(echo_server benchmarks/echo_server.cpp)
target_link_libraries(echo_server PRIVATE uringcore_core)

# Unit Tests
add_executable(test_lock_free_queue tests/unit/test_lock_free_queue.cpp)
target_link_libraries(test_lock_free_queue PRIVATE uringcore_core)

add_executable(test_worker_group tests/unit/test_worker_group.cpp)
target_link_libraries(test_worker_group PRIVATE uringcore_core)

add_executable(test_buffer_ring tests/unit/test_buffer_ring.cpp)
target_link_libraries(test_buffer_ring PRIVATE uringcore_core)

add_executable(test_sanitizer_teardown tests/unit/test_sanitizer_teardown.cpp)
target_link_libraries(test_sanitizer_teardown PRIVATE uringcore_core)

add_executable(test_worker_pool tests/unit/test_worker_pool.cpp)
target_link_libraries(test_worker_pool PRIVATE uringcore_core)
