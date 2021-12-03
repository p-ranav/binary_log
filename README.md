<p align="center">
  <img height="70" src="images/logo.png"/>  
</p>

# Highlights

* Logs messages in a compact binary format
  * ***Hundreds of millions*** of logs per second!
* Extracts static information at compile-time
* Only logs the dynamic parts of the messages at runtime in the hot path
* Provides an [unpacker](https://github.com/p-ranav/binary_log/tree/master/tools/unpacker) to deflate the log messages
* Synchronous logging - not thread safe
* Header-only library
* Requires C++20
* MIT License

# Usage and Performance

The following code logs 1 billion `uint64_t` integers to file.

```cpp
#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  for (uint64_t i = 0; i < 1E9; ++i)
    BINARY_LOG(log, "Hello logger, msg number: {}", i);
}
```

On a modern workstation desktop with an [ADATA SX8200PNP NVMe PCIe SSD](https://www.adata.com/upload/downloadfile/Datasheet_XPG%20SX8200%20Pro_EN_20181017.pdf), the above code executes in `~4.1s` and writes 2 files: a log file and an index file.

| Type            | Value                                       |
| --------------- | ------------------------------------------- |
| Time Taken      | 4.1 s                                       | 
| Throughput      | 1.465 Gb/s                                  |
| Performance     | 244 million logs/s                          |
| File Size       | ~6 GB (log file) + 32 bytes (index file)    |

```console
foo@bar:~/dev/binary_log$ time ./build/example/example

real    0m4.093s
user    0m2.672s
sys     0m1.422s

foo@bar:~/dev/binary_log$ ls -lart log.out*
-rw-r--r-- 1 pranav pranav         32 Dec  3 13:33 log.out.index
-rw-r--r-- 1 pranav pranav 5999868672 Dec  3 13:33 log.out
```

See [benchmarks](https://github.com/p-ranav/binary_log/blob/master/README.md#benchmarks) section for more performance metrics.

# Design Goals & Decisions

* Implement a single-threaded, synchronous logger - Do not provide thread safety
  - If the user wants multi-threaded behavior, the user can choose and implement their own queueing solution
  - There are numerous well-known lock-free queues available for this purpose ([moody::concurrentqueue](https://github.com/cameron314/concurrentqueue), [atomic_queue](https://github.com/max0x7ba/atomic_queue) etc.) - let the user choose the technology they want to use.
  - The latency of enqueuing into a lock-free queue is large enough to matter
    - Users who do not care about multi-threaded scenarios should NOT suffer the cost
    - Looking at the [atomic_queue benchmarks](https://max0x7ba.github.io/atomic_queue/html/benchmarks.html), the average latency across many state-of-the-art multi-producer, multi-consumer queues is around 150-250 ns
* Avoid writing static information (format string, and constants) more than once
  - Store the static information in an "index" file 
  - Store the dynamic information in the log file (refer to the index file where possible)
* Do as little work as possible in the runtime hot path
  - No formatting of any kind
  - All formatting will happen offline using an unpacker that deflates the binary logs

# Benchmarks

| Type            | Value                                                                                                     |
| --------------- | --------------------------------------------------------------------------------------------------------- |
| Processor       | 11th Gen Intel(R) Core(TM) i9-11900KF @ 3.50GHz   3.50 GHz                                                |
| Installed RAM   | 32.0 GB (31.9 GB usable)                                                                                  |
| SSD             | [ADATA SX8200PNP](https://www.adata.com/upload/downloadfile/Datasheet_XPG%20SX8200%20Pro_EN_20181017.pdf) |

```console
foo@bar:~/dev/binary_log$  ./build/benchmark/binary_log_benchmark --benchmark_counters_tabular=true
2021-12-03T13:42:15-06:00
Running ./build/benchmark/binary_log_benchmark
Run on (16 X 3504 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
-------------------------------------------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations    Bytes/s    Latency     Logs/s
-------------------------------------------------------------------------------------------------------
BM_binary_log_static_string      0.678 ns        0.672 ns   1000000000 1.48837G/s  671.875ps 1.48837G/s
BM_binary_log_constants          0.676 ns        0.680 ns    896000000 1.47036G/s  680.106ps 1.47036G/s
BM_binary_log_integer             2.05 ns         2.04 ns    344615385  2.4506G/s  2.04032ns  490.12M/s
BM_binary_log_double              6.14 ns         6.14 ns    112000000 1.46618G/s  6.13839ns 162.909M/s
BM_binary_log_string              12.2 ns         12.2 ns     64000000 1.39264G/s   12.207ns   81.92M/s
```

# Building and installing

See the [BUILDING](BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

<!--
Please go to https://choosealicense.com/ and choose a license that fits your
needs. GNU GPLv3 is a pretty nice option ;-)
-->
