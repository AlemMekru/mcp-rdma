# MCP-RDMA Preliminary Benchmark Results

## Environment
- Platform: Ubuntu VPS
- RDMA: Soft-RoCE (software RDMA)
- Test topology: Client and server on the same VPS
- Workload: Sequential 10-byte requests with numbered ACKs
- Warm-up: 100 exchanges per run
- Timing: Application-level round-trip latency

## 100,000-Exchange Benchmark

| Metric | TCP | Experimental RDMA |
|---|---:|---:|
| Median RTT | 64.50 us | 14.87 us |
| p95 RTT | 98.74 us | 29.65 us |
| p99 RTT | 120.16 us | 43.06 us |
| Client elapsed time | 7.34 s | 1.85 s |
| Combined CPU time | 8.01 s | 3.66 s |

Observed median RTT ratio: 4.34x (TCP / RDMA).

## Reproducibility
Three independent 1,000-exchange runs:
- TCP mean of run medians: 57.02 us
- RDMA mean of run medians: 12.94 us
- Observed ratio: 4.41x

## Limitations
- Same-host measurements, not distributed-network results
- Soft-RoCE, not hardware RDMA
- TCP uses blocking sockets; experimental RDMA uses busy polling
- Optimized RDMA uses separate SEND and RECEIVE completion queues
- No CPU-affinity or system-load controls documented
- Raw TCP and RDMA microbenchmarks, not full HTTP-based MCP
- Results do not establish clinical safety or DCMSP effectiveness
- Further independent repetitions and cross-host testing are required
