# binary_log

## Highlights

* Logs messages in a compact binary format
* Extracts static information at compile-time
* Only log the dynamic parts of the messages at runtime in the hot path
* An offline process can be used to deflate and format the log messages
* Requires C++20
* MIT License

```cpp
#include <binary_log/binary_log.hpp>

int main() {
  binary_log::binary_log log("log.out");
  
  constexpr auto how_many = 1E9; // 1 billion

  for (std::size_t i = 0; i < how_many; ++i)
    BINARY_LOG(log, "Hello {}", binary_log::constant("World"));
}

/*
 * The above code:
 *   Runs in ~980 ms (average latency less than 1 nanosecond!)
 *   Writes just 18 bytes
*/
```

## Design Goals & Decisions

* Design for embedded applications, e.g., ESP32
  - Log fast
  - Be space efficient
* Implement a single-threaded, synchronous logger - Do not provide thread safety
  - If the user wants multi-threaded behavior, the user can choose and implement their own queueing solution
  - There are numerous well-known lock-free queues available for this purpose ([moody::concurrentqueue](https://github.com/cameron314/concurrentqueue), [atomic_queue](https://github.com/max0x7ba/atomic_queue) etc.)
  - The latency of enqueuing into a lock-free queue is large enough to matter - Users who do not care about this should NOT suffer the cost.
* Avoid writing information that is not requested by the user - log level, timestamp etc.
* Avoid writing static information (format string, and constants) more than once
  - Store the static information in an "index" file 
  - Store the dynamic information in the log file (refer to the index file where possible)
* Do as little work as possible in the runtime hot path
  - No formatting of any kind
  - All formatting will happen offline using an unpacker that deflates the binary logs
* Compress the logged information as much as possible
  - Never at the cost of latency
  - Use run-length encoding, if possible, to encode the number of consecutive log calls made

## Benchmarks

```console
foo@bar:~/dev/binary_log$ ./build/benchmark/binary_log_benchmark
2021-11-28T22:43:22-06:00
Running ./build/benchmark/binary_log_benchmark
Run on (8 X 2611 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
---------------------------------------------------------------------------------------------
Benchmark                                                   Time             CPU   Iterations
---------------------------------------------------------------------------------------------
BM_binary_log_latency_static_string                     0.630 ns        0.625 ns   1000000000
BM_binary_log_latency_one_static_integer                0.646 ns        0.656 ns   1000000000
BM_binary_log_latency_two_static_integers               0.620 ns        0.609 ns   1000000000
BM_binary_log_latency_one_static_float                  0.615 ns        0.609 ns   1000000000
BM_binary_log_latency_one_static_double                 0.660 ns        0.672 ns   1000000000
BM_binary_log_latency_static_ints_and_doubles           0.633 ns        0.625 ns   1000000000
BM_binary_log_latency_static_cstring                    0.644 ns        0.645 ns    896000000
BM_binary_log_latency_static_std_string                  1.82 ns         1.80 ns    373333333
BM_binary_log_latency_static_std_string_view            0.623 ns        0.625 ns   1000000000
BM_binary_log_latency_random_integer                     32.1 ns         32.2 ns     21333333
BM_binary_log_latency_random_float                       23.5 ns         23.7 ns     26352941
BM_binary_log_latency_random_double                      32.5 ns         32.1 ns     22400000
BM_binary_log_latency_random_std_string_of_size/4        58.0 ns         56.2 ns     10000000
BM_binary_log_latency_random_std_string_of_size/8        89.0 ns         88.9 ns      8960000
BM_binary_log_latency_random_std_string_of_size/16        151 ns          150 ns      4072727
BM_binary_log_latency_random_std_string_of_size/32        270 ns          267 ns      2635294
BM_binary_log_latency_random_std_string_of_size/64        495 ns          488 ns      1120000
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
