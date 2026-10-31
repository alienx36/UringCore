import json
import os
import matplotlib.pyplot as plt

def main():
    json_path = os.path.expanduser("~/UringCore/telemetry_summary.json")
    if not os.path.exists(json_path):
        print(f"Error: {json_path} not found.")
        return

    with open(json_path, 'r') as f:
        data = json.load(f)

    variants = list(data.keys())
    rps = [data[v]["rps"] for v in variants]
    p99 = [data[v]["p99_us"] for v in variants]

    colors = ['#4C72B0', '#55A868', '#C44E52', '#8172B1', '#CCB974']

    # 1. Throughput Chart
    plt.figure(figsize=(8, 5))
    bars1 = plt.bar(variants, rps, color=colors)
    plt.title("Throughput Comparison (Requests/sec)", fontsize=12, fontweight='bold')
    plt.ylabel("Requests / sec")
    plt.xticks(range(len(variants)), variants, rotation=25, ha='right')
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    for bar in bars1:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval + (max(rps)*0.01), f"{int(yval):,}", ha='center', va='bottom', fontsize=9)

    plt.tight_layout()
    throughput_path = os.path.expanduser("~/UringCore/throughput_benchmark.png")
    plt.savefig(throughput_path, dpi=300)
    plt.close()

    # 2. p99 Tail Latency Chart
    plt.figure(figsize=(8, 5))
    bars2 = plt.bar(variants, p99, color=colors)
    plt.title("p99 Tail Latency (Microseconds - Lower is Better)", fontsize=12, fontweight='bold')
    plt.ylabel("p99 Latency (µs)")
    plt.xticks(range(len(variants)), variants, rotation=25, ha='right')
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    for bar in bars2:
        yval = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2, yval + (max(p99)*0.01), f"{yval:.1f}µs", ha='center', va='bottom', fontsize=9)

    plt.tight_layout()
    latency_path = os.path.expanduser("~/UringCore/latency_benchmark.png")
    plt.savefig(latency_path, dpi=300)
    plt.close()

    print(f"[+] Individual charts saved:")
    print(f"    - {throughput_path}")
    print(f"    - {latency_path}")

if __name__ == "__main__":
    main()
