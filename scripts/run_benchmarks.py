import asyncio
import time
import statistics
import argparse

async def send_worker_requests(port, payload, requests_per_worker, latency_list):
    try:
        reader, writer = await asyncio.open_connection('127.0.0.1', port)
        payload_len = len(payload)
        
        for _ in range(requests_per_worker):
            start = time.perf_counter()
            writer.write(payload)
            await writer.drain()

            received = 0
            while received < payload_len:
                data = await reader.read(payload_len - received)
                if not data:
                    break
                received += len(data)
            
            if received == payload_len:
                elapsed_ms = (time.perf_counter() - start) * 1000.0
                latency_list.append(elapsed_ms)
            else:
                break
            
        writer.close()
        await writer.wait_closed()
    except Exception:
        pass

async def main():
    parser = argparse.ArgumentParser(description="UringCore Latency & Throughput Benchmark")
    parser.add_argument("--port", type=int, default=8080, help="Target echo server port")
    parser.add_argument("--concurrency", type=int, default=50, help="Number of concurrent connections")
    parser.add_argument("--requests", type=int, default=10000, help="Total requests to transmit")
    args = parser.parse_args()

    payload = b"UringCore High Performance TCP Test Payload 1234567890"
    requests_per_worker = args.requests // args.concurrency
    latencies = []

    print("=========================================")
    print(" Starting UringCore Load Test Harness")
    print(f" Target Port:  {args.port}")
    print(f" Concurrency:  {args.concurrency}")
    print(f" Total Req:    {args.requests}")
    print("=========================================")

    start_time = time.perf_counter()

    tasks = [
        asyncio.create_task(send_worker_requests(args.port, payload, requests_per_worker, latencies))
        for _ in range(args.concurrency)
    ]
    await asyncio.gather(*tasks)

    total_time = time.perf_counter() - start_time
    completed = len(latencies)
    rps = completed / total_time if total_time > 0 else 0

    if completed > 0:
        latencies.sort()
        p50 = latencies[int(completed * 0.50)]
        p90 = latencies[int(completed * 0.90)]
        p99 = latencies[int(completed * 0.99)]
    else:
        p50 = p90 = p99 = 0.0

    print("\n================ RESULTS ================")
    print(f" Total Completed: {completed} / {args.requests}")
    print(f" Elapsed Time:    {total_time:.2f} s")
    print(f" Throughput:      {rps:.2f} req/sec")
    print(f" p50 Latency:     {p50:.3f} ms")
    print(f" p90 Latency:     {p90:.3f} ms")
    print(f" p99 Latency:     {p99:.3f} ms")
    print("=========================================\n")

if __name__ == "__main__":
    asyncio.run(main())
