# binary_log

## Motivation

Consider the example:

```cpp
for (std::size_t i = 0; i < 1E6; ++i) {
  logger.log("Hello World");
}
```

Most state-of-the-art loggers will output:
* 1 million lines
* At least 11 MB (spdlog basic_logger writes out 60MB with timestamp and log level)
* Takes hundreds of milliseconds (spdlog takes ~500ms)

## Better packing

`binary_log` extracts static log information (format string, number of args, which args are constant, etc.) at compile-time and logs them to an index file. Only the dynamic parts of the log call are logged to a log file in the runtime hot path. The formatting is deferred to an offline process.

```cpp
#include <binary_log/binary_log.hpp>

int main() {
  binary_log::binary_log log("log.out");

  for (std::size_t i = 0; i < 1E6; ++i) {
    BINARY_LOG(log, "Hello World");
  }
}
```

* Two files are written: `log.out` and `log.out.index`
* File Size: 18 bytes
* Time taken: 15.8 ms 
* Average latency after the first call: 1.7 ns

## Benchmarks

```console
foo@bar:~/dev/binary_log$ ./build/benchmark/binary_log_benchmark
2021-11-28T20:20:08-06:00
Running ./build/benchmark/binary_log_benchmark
Run on (8 X 2611 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
---------------------------------------------------------------------------------------------
Benchmark                                                   Time             CPU   Iterations
---------------------------------------------------------------------------------------------
BM_binary_log_latency_random_integer                     29.0 ns         28.9 ns     24888889
BM_binary_log_latency_random_float                       22.0 ns         22.0 ns     32000000
BM_binary_log_latency_random_double                      31.6 ns         31.8 ns     23578947
BM_binary_log_latency_random_std_string_of_size/4        57.7 ns         57.2 ns     11200000
BM_binary_log_latency_random_std_string_of_size/8        85.6 ns         85.4 ns      8960000
BM_binary_log_latency_random_std_string_of_size/16        148 ns          146 ns      4480000
BM_binary_log_latency_random_std_string_of_size/32        266 ns          267 ns      2635294
BM_binary_log_latency_random_std_string_of_size/64        487 ns          487 ns      1445161
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
