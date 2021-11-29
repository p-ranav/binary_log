# binary_log

## Highlights

* Logs messages in a compact binary format
  * Average Latency:
      * ~9 ns for static data
      * ~20 ns for integers, floats, and doubles 
      * ~260 ns for random 32-byte strings
* Extracts static information at compile-time
* Only log the dynamic parts of the messages at runtime in the hot path
* An offline process can be used to deflate and format the log messages
* Requires C++20
* MIT License

```cpp
#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  for (uint32_t i = 0; i < 1E6; ++i)
    BINARY_LOG(log, "Hello logger, msg number: {}", i);
}

/*
 * The above code:
 *   Runs in ~40 ms
 *   Average latency is ~23 ns
 *   File size: 5 MB
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
2021-11-29T06:05:23-06:00
Running ./build/benchmark/binary_log_benchmark
Run on (8 X 2611 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
---------------------------------------------------------------------------------------------
Benchmark                                                   Time             CPU   Iterations
---------------------------------------------------------------------------------------------
BM_binary_log_latency_static_string                      8.91 ns         9.00 ns     74666667
BM_binary_log_latency_one_static_integer                 9.20 ns         9.21 ns     74666667
BM_binary_log_latency_two_static_integers                9.09 ns         9.07 ns     89600000
BM_binary_log_latency_one_static_float                   9.00 ns         8.89 ns     89600000
BM_binary_log_latency_one_static_double                  8.95 ns         9.00 ns     74666667
BM_binary_log_latency_static_ints_and_doubles            9.33 ns         9.42 ns     74666667
BM_binary_log_latency_static_cstring                     9.54 ns         9.59 ns     89600000
BM_binary_log_latency_static_std_string                  10.5 ns         10.3 ns     64000000
BM_binary_log_latency_static_std_string_view             9.10 ns         9.00 ns     74666667
BM_binary_log_latency_random_integer                     28.7 ns         28.6 ns     21333333
BM_binary_log_latency_random_float                       22.8 ns         22.5 ns     26352941
BM_binary_log_latency_random_double                      30.4 ns         30.8 ns     24888889
BM_binary_log_latency_random_std_string_of_size/4        58.6 ns         58.6 ns     11200000
BM_binary_log_latency_random_std_string_of_size/8        85.9 ns         83.7 ns      8960000
BM_binary_log_latency_random_std_string_of_size/16        150 ns          150 ns      4072727
BM_binary_log_latency_random_std_string_of_size/32        266 ns          261 ns      2635294
BM_binary_log_latency_random_std_string_of_size/64        491 ns          500 ns      1000000
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
