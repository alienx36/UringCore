import os
import re
import json

LOG_FILES = {
    "epoll_baseline": "epoll_baseline.log",
    "uring_single_thread": "uring_single_thread.log",
    "uring_multi_mutex": "uring_multi_mutex.log",
    "uring_multi_lockfree": "uring_multi_lockfree.log",
    "uring_registered_buffers": "uring_registered_buffers.log"
}

def parse_time_us(time_str, unit):
    val = float(time_str)
    unit = unit.lower()
    if unit == 'us':
        return val
    elif unit == 'ms':
        return val * 1000.0
    elif unit == 's':
        return val * 1000000.0
    return val

def parse_wrk_log(filepath):
    if not os.path.exists(filepath):
        return None

    with open(filepath, 'r') as f:
        content = f.read()

    metrics = {
        "rps": 0.0,
        "latency_avg_us": 0.0,
        "latency_stdev_us": 0.0,
        "latency_max_us": 0.0,
        "p50_us": 0.0,
        "p75_us": 0.0,
        "p90_us": 0.0,
        "p99_us": 0.0
    }

    # Extract Requests/sec
    rps_match = re.search(r'Requests/sec:\s+([\d.]+)', content)
    if rps_match:
        metrics["rps"] = float(rps_match.group(1))

    # Extract Latency Line (Avg, Stdev, Max)
    lat_match = re.search(r'Latency\s+([\d.]+)(us|ms|s)\s+([\d.]+)(us|ms|s)\s+([\d.]+)(us|ms|s)', content)
    if lat_match:
        metrics["latency_avg_us"] = parse_time_us(lat_match.group(1), lat_match.group(2))
        metrics["latency_stdev_us"] = parse_time_us(lat_match.group(3), lat_match.group(4))
        metrics["latency_max_us"] = parse_time_us(lat_match.group(5), lat_match.group(6))

    # Extract Percentiles
    for line in content.splitlines():
        perc_match = re.search(r'^\s*(\d+)%\s+([\d.]+)(us|ms|s)', line)
        if perc_match:
            pct = int(perc_match.group(1))
            val_us = parse_time_us(perc_match.group(2), perc_match.group(3))
            if pct == 50:
                metrics["p50_us"] = val_us
            elif pct == 75:
                metrics["p75_us"] = val_us
            elif pct == 90:
                metrics["p90_us"] = val_us
            elif pct == 99:
                metrics["p99_us"] = val_us

    return metrics

def main():
    results = {}
    build_dir = os.path.expanduser("~/UringCore/build")
    
    for variant, filename in LOG_FILES.items():
        path = os.path.join(build_dir, filename)
        parsed = parse_wrk_log(path)
        if parsed:
            results[variant] = parsed
        else:
            print(f"[!] Warning: Log file missing or unparseable at {path}")

    print(json.dumps(results, indent=2))
    
    output_path = os.path.expanduser("~/UringCore/telemetry_summary.json")
    with open(output_path, 'w') as f:
        json.dump(results, f, indent=2)
    print(f"\n[+] Telemetry summary successfully saved to {output_path}")

if __name__ == "__main__":
    main()
