import subprocess
import time
import os

VARIANTS = [
    ("epoll_baseline", "./epoll_baseline", "http://127.0.0.1:8080/"),
    ("uring_single_thread", "./uring_single_thread", "http://127.0.0.1:8081/"),
    ("uring_multi_mutex", "./uring_multi_mutex", "http://127.0.0.1:8080/"),
    ("uring_multi_lockfree", "./uring_multi_lockfree", "http://127.0.0.1:8083/"),
    ("uring_registered_buffers", "./uring_registered_buffers", "http://127.0.0.1:8084/")
]

def main():
    build_dir = os.path.expanduser("~/UringCore/build")
    os.chdir(build_dir)

    print("[*] Starting Automated UringCore Benchmark Pipeline...")

    for name, bin_path, url in VARIANTS:
        print(f"\n[+] Launching and testing: {name}")
        server = subprocess.Popen([bin_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(1) # Allow socket to bind

        log_file = f"{name}.log"
        wrk_cmd = f"wrk -t4 -c50 -d10s --latency {url} > {log_file}"
        subprocess.run(wrk_cmd, shell=True)

        server.terminate()
        server.wait()
        print(f"    - Complete. Output saved to {log_file}")

    print("\n[*] Executing telemetry parser and chart generator...")
    subprocess.run(["python3", os.path.expanduser("~/UringCore/scripts/parse_latency.py")])
    subprocess.run(["python3", os.path.expanduser("~/UringCore/scripts/generate_charts.py")])
    print("\n[✔] Full 5-variant benchmark suite executed and documented successfully!")

if __name__ == "__main__":
    main()
