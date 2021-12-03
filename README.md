<p align="center">
  <img height="70" src="images/logo.png"/>  
</p>

## Highlights

* Logs messages in a compact binary format
  * ***Hundreds of millions*** of logs per second!
* Extracts static information at compile-time
* Only logs the dynamic parts of the messages at runtime in the hot path
* Provides an [unpacker](https://github.com/p-ranav/binary_log/tree/master/tools/unpacker) to deflate the log messages
* Synchronous logging - not thread safe
* Header-only library
* Requires C++20
* MIT License

## Performance

On a modern workstation desktop with an [ADATA SX8200PNP NVMe PCIe SSD](https://www.adata.com/upload/downloadfile/Datasheet_XPG%20SX8200%20Pro_EN_20181017.pdf), the following code 
* Runs in ~4 ms
* ~333 million logs per second
* Throughput of ~1.67 GB/s
* Average latency of ~3.64 ns

```cpp
#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  for (uint32_t i = 0; i < 1E6; ++i)
    BINARY_LOG(log, "Hello logger, msg number: {}", i);
}
```

## Design Goals & Decisions

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

## Benchmarks

| Type            | Value                                                                                                     |
| --------------- | --------------------------------------------------------------------------------------------------------- |
| Processor       | 11th Gen Intel(R) Core(TM) i9-11900KF @ 3.50GHz   3.50 GHz                                                |
| Installed RAM   | 32.0 GB (31.9 GB usable)                                                                                  |
| SSD             | [ADATA SX8200PNP](https://www.adata.com/upload/downloadfile/Datasheet_XPG%20SX8200%20Pro_EN_20181017.pdf) |

```console
foo@bar:~/dev/binary_log$  ./build/benchmark/binary_log_benchmark --benchmark_counters_tabular=true
2021-12-03T08:03:14-06:00
Running ./build/benchmark/binary_log_benchmark
Run on (16 X 3504 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
-------------------------------------------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations    Bytes/s    Latency     Logs/s
-------------------------------------------------------------------------------------------------------
BM_binary_log_static_string      0.678 ns        0.672 ns   1000000000 1.48837G/s  671.875ps 1.48837G/s
BM_binary_log_integer             3.36 ns         3.37 ns    213333333 1.48406G/s  3.36914ns 296.812M/s
BM_binary_log_double              6.09 ns         6.14 ns    112000000 1.46618G/s  6.13839ns 162.909M/s
BM_binary_log_string              8.81 ns         8.79 ns     74666667 1.47911G/s  8.78906ns 113.778M/s
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
