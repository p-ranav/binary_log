# binary_log

* Logs messages in compact binary format
* Extracts static information at compile-time and logs them to an index file
* Only dynamic parts of the logs are written in the runtime hot path
* Run-length encoding is used, where possible, to encode repeated log calls
* An offline process can be used to deflate and format the log messages
* Average latency (measured on a Surface Laptop 4, Ubuntu in WSL)
  - ~9 ns for static data
  - ~30 ns for random integers, floats, doubles
  - ~270 ns for random 32-byte strings

The following code completes in ~980 ms and writes just 18 bytes. The average latency of the log call is under 1 ns!

```cpp
#include <binary_log/binary_log.hpp>

int main() {
  binary_log::binary_log log("log.out");

  for (std::size_t i = 0; i < 1E9; ++i) {
    BINARY_LOG(log, "Hello {}", binary_log::constant("World"));
  }
}
```

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
