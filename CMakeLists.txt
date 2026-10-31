cmake_minimum_required(VERSION 3.25)
project(UringCore CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -pthread")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBURING REQUIRED liburing)

include_directories(include ${LIBURING_INCLUDE_DIRS})

file(GLOB_RECURSE CORE_SOURCES "src/*.cpp")

# Variant 1: POSIX Baseline
add_executable(epoll_baseline benchmarks/posix_baseline.cpp ${CORE_SOURCES})
target_link_libraries(epoll_baseline ${LIBURING_LIBRARIES} pthread)

# Variant 2: Uring Single Thread
add_executable(uring_single_thread benchmarks/uring_single_thread.cpp ${CORE_SOURCES})
target_link_libraries(uring_single_thread ${LIBURING_LIBRARIES} pthread)

# Variant 3: Uring Multi Mutex
add_executable(uring_multi_mutex benchmarks/uring_throughput.cpp ${CORE_SOURCES})
target_link_libraries(uring_multi_mutex ${LIBURING_LIBRARIES} pthread)

# Variant 4: Uring Multi Lockfree
add_executable(uring_multi_lockfree benchmarks/uring_multi_lockfree.cpp ${CORE_SOURCES})
target_link_libraries(uring_multi_lockfree ${LIBURING_LIBRARIES} pthread)

# Variant 5: Uring Registered Buffers
add_executable(uring_registered_buffers benchmarks/uring_registered_buffers.cpp ${CORE_SOURCES})
target_link_libraries(uring_registered_buffers ${LIBURING_LIBRARIES} pthread)
