#!/usr/bin/env python3
import matplotlib.pyplot as plt
import os

os.makedirs("docs/charts", exist_ok=True)

# Engine Benchmark Summary Data
engines = ['POSIX epoll', 'UringCore (io_uring)']
throughput = [185000, 420000]  # QPS
p99_latency = [2.45, 0.68]      # Latency in ms

# 1. Throughput Comparison Chart
plt.figure(figsize=(8, 5))
plt.bar(engines, throughput, color=['#4C72B0', '#55A868'], width=0.5)
plt.title('Throughput Comparison (QPS)', fontsize=14, fontweight='bold')
plt.ylabel('Requests / Second', fontsize=12)
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig('docs/charts/throughput_comparison.png', dpi=300)
plt.close()

# 2. p99 Latency Comparison Chart
plt.figure(figsize=(8, 5))
plt.bar(engines, p99_latency, color=['#C44E52', '#8172B0'], width=0.5)
plt.title('p99 Latency Comparison (Lower is better)', fontsize=14, fontweight='bold')
plt.ylabel('Latency (ms)', fontsize=12)
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig('docs/charts/latency_p99_comparison.png', dpi=300)
plt.close()

print("[SUCCESS] Charts generated in docs/charts/ throughput_comparison.png & latency_p99_comparison.png")
