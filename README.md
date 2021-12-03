<p align="center">
  <img height="70" src="images/logo.png"/>  
</p>

## Highlights

* Logs messages in a compact binary format
* 100-200 million logs per second!
* Extracts static information at compile-time
* Only logs the dynamic parts of the messages at runtime in the hot path
* Provides an [unpacker](https://github.com/p-ranav/binary_log/tree/master/tools/unpacker) to deflate the log messages
* Synchronous logging - not thread safe
* Header-only library
* Requires C++20
* MIT License

## Performance

On a Surface Laptop 4, the following code 
* Runs in ~5 ms
* ~200 million logs per second
* Throughput of ~1 GB/s
* Average latency of ~4.5 ns

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

```console
foo@bar:~/dev/binary_log$  ./build/benchmark/binary_log_benchmark --benchmark_counters_tabular=true
2021-12-02T23:02:36-06:00
Running ./build/benchmark/binary_log_benchmark
Run on (8 X 2611 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
-------------------------------------------------------------------------------------------------------
Benchmark                            Time             CPU   Iterations    Bytes/s    Latency     Logs/s
-------------------------------------------------------------------------------------------------------
BM_binary_log_static_string      0.858 ns        0.858 ns    746666667 1.16553G/s   857.98ps 1.16553G/s
BM_binary_log_integer             4.62 ns         4.59 ns    160000000 1089.36M/s  4.58984ns 217.872M/s
BM_binary_log_double              7.81 ns         7.67 ns     89600000 1.17295G/s  7.67299ns 130.327M/s
BM_binary_log_string              11.3 ns         11.2 ns     64000000 1.15757G/s  11.2305ns 89.0435M/s
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
