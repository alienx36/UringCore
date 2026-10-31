# UringCore Empirical Performance Analysis

This document provides a comprehensive latency distribution, system-level trade-off analysis, and hardware performance comparison for all 5 engine variants implemented in `UringCore`.

---

## 1. Empirical Results Summary Table

Data captured via `wrk` (4 threads, 50 concurrent connections, 10s duration) on Linux:

| Engine Architecture | Port | Throughput (RPS) | p50 (µs) | p75 (µs) | p90 (µs) | p99 (µs) | Max Latency |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **`epoll_baseline`** | 8080 | 74,010.57 | 617.0 | 727.0 | 845.0 | **1,450.0** | 11.55 ms |
| **`uring_single_thread`** | 8081 | 59,225.63 | 801.0 | 950.0 | 1100.0 | **4,830.0** | 31.91 ms |
| **`uring_multi_mutex`** | 8080 | **195,862.71** | 175.0 | 236.0 | 391.0 | **1,250.0** | 31.38 ms |
| **`uring_multi_lockfree`** | 8083 | 193,470.89 | 175.0 | 233.0 | 431.0 | **10,870.0** | 54.13 ms |
| **`uring_registered_buffers`** | 8084 | 79,438.28 | 602.0 | 672.0 | 743.0 | **890.0** | 11.00 ms |

---

## 2. Low-Level System Trade-Offs

### A. epoll vs. Single-Threaded io_uring
* **System Call Overhead:** `epoll_baseline` requires system calls per event batch (`epoll_wait` + I/O execution). `uring_single_thread` processes completions asynchronously via submission/completion rings without readiness polling context switches.

### B. Mutex vs. Lock-Free Work Distribution
* **Throughput & Contention:** `uring_multi_mutex` achieved the peak throughput (**195,862.71 RPS**) and tighter tail latency (p99 of **1,250.0 µs**) under current thread loads. High core concurrency on single-producer/multi-consumer lock-free atomics can introduce cache line bouncing under severe memory contention, driving p99 latency spikes up to **10,870.0 µs**.

### C. Kernel Buffer Registration
* **Fixed Buffers:** Pre-registering fixed kernel buffers (`io_uring_register_buffers`) avoids page pinning and dynamic buffer allocations during ring operations, producing the most stable tail latency profile with the **lowest overall p99 tail latency (890.0 µs)** and a low standard deviation (**184.77 µs**).
