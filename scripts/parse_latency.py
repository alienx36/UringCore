#!/usr/bin/env python3
import sys
import re
import json

def parse_wrk_latency(file_path):
    metrics = {
        "p50": None,
        "p90": None,
        "p99": None,
        "p99_9": None,
        "qps": None
    }
    
    with open(file_path, 'r') as f:
        content = f.read()

    # Match QPS / Requests per second
    qps_match = re.search(r'Requests/sec:\s+([\d\.]+)', content)
    if qps_match:
        metrics["qps"] = float(qps_match.group(1))

    # Match Percentiles
    p50_match = re.search(r'50%\s+([\d\.]+)(us|ms|s)', content)
    p90_match = re.search(r'90%\s+([\d\.]+)(us|ms|s)', content)
    p99_match = re.search(r'99%\s+([\d\.]+)(us|ms|s)', content)
    p99_9_match = re.search(r'99\.9%\s+([\d\.]+)(us|ms|s)', content)

    def to_ms(value, unit):
        val = float(value)
        if unit == 'us':
            return val / 1000.0
        elif unit == 's':
            return val * 1000.0
        return val

    if p50_match:
        metrics["p50"] = to_ms(p50_match.group(1), p50_match.group(2))
    if p90_match:
        metrics["p90"] = to_ms(p90_match.group(1), p90_match.group(2))
    if p99_match:
        metrics["p99"] = to_ms(p99_match.group(1), p99_match.group(2))
    if p99_9_match:
        metrics["p99_9"] = to_ms(p99_9_match.group(1), p99_9_match.group(2))

    return metrics

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 parse_latency.py <wrk_output.log>")
        sys.exit(1)
        
    data = parse_wrk_latency(sys.argv[1])
    print(json.dumps(data, indent=2))
