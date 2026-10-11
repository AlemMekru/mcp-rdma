# MCP-RDMA
### Accelerating AI Agent Communication with RDMA

**MCP-RDMA** is an experimental C-based research project investigating the use of **Remote Direct Memory Access (RDMA)** to reduce communication latency in AI-agent systems that exchange Model Context Protocol (MCP)-style messages.

The project combines RDMA networking with JSON-RPC 2.0 request-response processing, creating a foundation for studying low-latency communication, distributed agent coordination, and safety-critical AI systems.

**Research focus:** Low-latency communication and deadline-aware memory consistency in multi-agent AI.

**Project status:** Experimental implementation with working bidirectional RDMA messaging, preliminary benchmarks, and automated integration tests.

---

## 1. Key Features

The current implementation supports:

- RDMA client-server connection establishment
- Reliable Connected (RC) Queue Pairs
- RDMA memory registration
- SEND/RECV operations using `libibverbs`
- Bidirectional messaging
- Application-level acknowledgment-based flow control
- JSON-RPC 2.0 request-response processing
- Experimental MCP `server/discover` handling
- Standard JSON-RPC error responses
- Automated unit and integration testing
- Experimental latency benchmarking using Soft-RoCE

**Validation:** Five automated tests passed on the configured Ubuntu RDMA environment.

The implementation is intended for research and experimentation. It is not yet a production-ready or fully interoperable MCP transport.

---

## 2. Preliminary Performance Results

Initial experiments were conducted on an Ubuntu VPS using **Soft-RoCE (software RDMA)**.

A conventional TCP implementation was compared with an experimental RDMA configuration using busy polling and separate SEND and RECEIVE completion queues.

Both implementations performed 100,000 sequential request-acknowledgment exchanges following 100 warm-up exchanges.

### Round-Trip Latency

| Metric | Conventional TCP | Experimental MCP-RDMA |
|---|---:|---:|
| Median (p50) | 64.50 µs | **14.87 µs** |
| p95 | 98.74 µs | **29.65 µs** |
| p99 | 120.16 µs | **43.06 µs** |

### Key Observation

**The experimental RDMA implementation achieved 4.34× lower median RTT than the TCP baseline, expressed as a TCP/RDMA latency ratio, in the 100,000-exchange benchmark.**

This corresponds to approximately **76.9% lower median round-trip latency** under the tested conditions.

### Reproducibility

Three additional independent runs were performed for each transport, with 1,000 measured exchanges per run.

| Transport | Mean of Three Run Medians |
|---|---:|
| Conventional TCP | 57.02 µs |
| Experimental MCP-RDMA | **12.94 µs** |

The corresponding observed TCP/RDMA latency ratio was **4.41×**.

### Workload Completion and CPU Cost

In the 100,000-exchange experiment:

| Metric | TCP | Experimental MCP-RDMA |
|---|---:|---:|
| Client elapsed time | 7.34 s | **1.85 s** |
| Combined client/server CPU time | 8.01 s | **3.66 s** |

Although the RDMA workload completed faster and used less total measured CPU time in this experiment, the busy-polling client approached 100% CPU utilization while active.

### Important Experimental Limitations

These measurements must be interpreted within the following conditions:

- Client and server ran on the **same Ubuntu VPS**.
- RDMA used **Soft-RoCE**, not dedicated RDMA hardware.
- The TCP benchmark used blocking sockets with `TCP_NODELAY`.
- The optimized RDMA benchmark used busy polling and separate completion queues.
- The benchmark workload used a 10-byte request and numbered acknowledgments.
- The measurements compare specific transport implementations, not equivalent full MCP-over-HTTP stacks.
- Hardware differences, CPU scheduling, background system activity, and network topology may materially influence results.
- CPU affinity, cross-host measurements, and energy consumption have not yet been evaluated systematically.
- The optimized busy-polling implementation is currently experimental and separate from the main RDMA library.

**These results do not establish that RDMA universally outperforms TCP or that MCP-RDMA improves clinical safety.**

Further benchmarking is planned.

---

## 3. Architecture

The current experimental communication architecture is:

```text
             AI Agent / Client
                     |
                     v
               JSON-RPC 2.0
                     |
                     v
              MCP-RDMA Layer
                     |
                     v
         RDMA Connection Manager
                     |
                     v
       Reliable Connected Queue Pair
                     |
             +-------+-------+
             |               |
         SEND Queue      RECEIVE Queue
             |               |
             +-------+-------+
                     |
                     v
          Registered RDMA Memory
                     |
                     v
             RDMA / Soft-RoCE
                     |
                     v
               Server Agent
                     |
                     v
          JSON-RPC Request Handler
                     |
                     v
           JSON-RPC Response / ACK
```

The experimental busy-polling implementation uses separate SEND and RECEIVE completion queues to prevent unrelated completion events from interfering with message processing.

---

## 4. Technology Stack

| Component | Technology |
|---|---|
| Primary language | C11 |
| RDMA verbs | libibverbs |
| RDMA connection management | librdmacm |
| JSON processing | json-c |
| Build system | CMake |
| Testing | CTest |
| Development environment | Ubuntu Linux |
| RDMA test environment | Soft-RoCE |
| Benchmarking | C, Python, Linux timing tools |

---

## 5. Build and Installation

### Requirements

- Linux
- RDMA-capable network interface or configured Soft-RoCE
- GCC or compatible C11 compiler
- CMake 3.16+
- libibverbs
- librdmacm
- json-c

### Install Dependencies

On Ubuntu:

```bash
sudo apt-get update

sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libibverbs-dev \
    librdmacm-dev \
    libjson-c-dev
```

### Clone the Repository

```bash
git clone https://github.com/AlemMekru/mcp-rdma.git
cd mcp-rdma
```

### Build

```bash
cmake -S . -B build
cmake --build build
```

### Run Tests

```bash
ctest --test-dir build --output-on-failure
```

RDMA integration tests require a working RDMA device and appropriate network configuration.

To enable the optional RDMA integration tests:

```bash
cmake -S . -B build \
    -DMCP_RDMA_TEST_HOST=<server-ip>

cmake --build build

ctest --test-dir build --output-on-failure
```

---

## 6. Experimental Benchmarking

The benchmarking work investigates:

1. Communication round-trip latency
2. Median, p95, and p99 latency
3. Polling interval sensitivity
4. Busy polling versus sleep-based polling
5. Completion queue architecture
6. CPU consumption
7. Performance relative to conventional TCP

### Polling Optimization

Initial experiments showed that polling strategy strongly affects latency.

| RDMA Configuration | Observed Median RTT |
|---|---:|
| 1 ms sleep-based polling | 1,116.77 µs |
| 100 µs sleep-based polling | 180.99 µs |
| Busy polling with separate completion queues | 14.87 µs |

These are results from different experimental configurations. They should not be interpreted as isolating the effect of polling interval alone.

The next experimental phase will investigate:

- Reproducibility under controlled CPU load
- Cross-host communication
- Hardware RDMA
- Larger message sizes
- Concurrent communication
- Full HTTP-based MCP comparisons
- Throughput and tail-latency behavior

Benchmark source code and detailed measurement artifacts are being organized for reproducible publication.

---

## 7. Research Direction

This project contributes to an ongoing Applied AI doctoral research direction:

**When Milliseconds Matter: MCP-RDMA for Low-Latency Communication and Deadline-Aware Memory Consistency in Safety-Critical Medical AI Agents**

**Researcher:** Alemzewd Mekru Alemayehu  
**Institution:** Alma Mater Europaea

### Research Problem

As AI systems evolve toward autonomous and semi-autonomous multi-agent architectures in healthcare, communication latency and inconsistent shared memory can create challenges for time-critical decision support.

A message can arrive quickly while still containing outdated information.

Therefore, low-latency communication alone does not guarantee safe decision-making.

### Proposed Research Architecture

The broader research investigates combining:

- **MCP:** Standardized AI-agent interaction and context exchange
- **RDMA:** Low-latency communication mechanisms
- **DCMSP:** Deadline-Aware Clinical Memory Safety Protocol

DCMSP is a proposed safety-validation mechanism designed to examine:

- Clinical information freshness
- State-version consistency
- Dependencies between agent observations
- Decision deadlines
- Conditions requiring clinician escalation

**DCMSP remains a research proposal. Its implementation and safety effectiveness have not yet been demonstrated.**

### Planned Experimental Comparison

The research proposes evaluating four configurations:

| Configuration | Communication | Memory Validation |
|---|---|---|
| Baseline 1 | Conventional MCP transport | Without DCMSP |
| Baseline 2 | MCP-RDMA | Without DCMSP |
| Experimental 1 | Conventional MCP transport | With DCMSP |
| Experimental 2 | MCP-RDMA | With DCMSP |

Planned evaluation metrics include:

- Communication latency
- Deadline violations
- Stale-state acceptance
- Memory consistency failures
- Unnecessary human escalations
- Reliability under controlled communication delays

The purpose is to investigate the relationship between communication performance and information validity in safety-critical multi-agent systems.

The project does not provide clinical advice or autonomous treatment decisions.

---

## 8. Current Limitations and Future Work

The current implementation remains experimental.

Important future work includes:

- Robust message framing for larger payloads
- Full MCP protocol interoperability
- Concurrent client connections
- Improved connection lifecycle handling
- Additional recovery and fault-tolerance mechanisms
- Benchmark automation and reproducibility improvements
- Hardware-RDMA measurements
- Cross-host testing
- CPU and energy efficiency studies
- Deadline-aware memory-validation research

---

## 9. Project Status

**Current milestone:** Working experimental RDMA communication with bidirectional JSON-RPC request-response exchange and preliminary performance benchmarks.

**Next milestone:** Reproducible cross-transport benchmarking and broader MCP interoperability testing.

The long-term objective is to investigate whether RDMA-based communication and deadline-aware information validation can improve the performance and reliability of distributed AI-agent systems.

---

## 10. Author

**Alemzewd Mekru Alemayehu (Alem Mekru)**

Applied AI Doctoral Researcher — Alma Mater Europaea

- GitHub: https://github.com/AlemMekru
- LinkedIn: https://www.linkedin.com/in/alemmekru/

Research interests include:

- Applied Artificial Intelligence
- Multi-Agent AI Systems
- Model Context Protocol
- RDMA and Low-Latency Systems
- Distributed AI Architectures
- Explainable and Safety-Critical AI

---

## 11. Research Collaboration

Technical discussions, reproducibility feedback, and research collaboration are welcome.

Please use GitHub Issues to report technical problems, propose improvements, or discuss the experimental methodology.

**Repository:** https://github.com/AlemMekru/mcp-rdma

---

*Disclaimer: MCP-RDMA is an experimental research implementation. Preliminary transport benchmarks do not constitute evidence of clinical effectiveness, patient safety, or production readiness.*
