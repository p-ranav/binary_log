<p align="center">
  <img height="60" src="images/logo.png"/>  
</p>

# Highlights

* Logs messages in a compact binary format
* Fast
  * ***Hundreds of millions*** of logs per second
  * 2-6 ns average latency on basic data types
* Extracts static information at compile-time
* Only logs the dynamic parts of the messages at runtime in the hot path
* Provides an [unpacker](https://github.com/p-ranav/binary_log/tree/master/tools/unpacker) to deflate the log messages
* Uses [fmtlib](https://github.com/fmtlib/fmt) to format the logs
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
| Average Latency | 4.1 ns                                      |
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

## Deflate the logs

These binary log files can be deflated using the provided [unpacker](https://github.com/p-ranav/binary_log/tree/master/tools/unpacker) app:

```console
foo@bar:~/dev/binary_log$ ./build/tools/unpacker/unpacker -i log.out.index -l log.out > log.deflated

foo@bar:~/dev/binary_log$ wc -l log.deflated
1000000000 log.deflated

foo@bar:~/dev/binary_log$ $ head log.deflated
Hello logger, msg number: 0
Hello logger, msg number: 1
Hello logger, msg number: 2
Hello logger, msg number: 3
Hello logger, msg number: 4
Hello logger, msg number: 5
Hello logger, msg number: 6
Hello logger, msg number: 7
Hello logger, msg number: 8
Hello logger, msg number: 9

foo@bar:~/dev/binary_log$ tail log.deflated
Hello logger, msg number: 999999990
Hello logger, msg number: 999999991
Hello logger, msg number: 999999992
Hello logger, msg number: 999999993
Hello logger, msg number: 999999994
Hello logger, msg number: 999999995
Hello logger, msg number: 999999996
Hello logger, msg number: 999999997
Hello logger, msg number: 999999998
Hello logger, msg number: 999999999

foo@bar:~/dev/binary_log$ ls -lart log.deflated
-rw-r--r-- 1 pranav pranav 35888888890 Dec  3 18:09 log.deflated
```

| Type                | Value     |
| ------------------- | --------- |
| Time Taken          | 2m 33s    | 
| Throughput          | 234 MB/s  |
| Deflated File Size  | ~35 GB    |

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

# Binary Format

Consider the following log:

```cpp
BINARY_LOG(log, "[Thread {}] [{}] Motors enabled, velocity = {}", 
           thread_id,
           component_name,
           velocity_rpm);
```

The above call can be deconstructed as follows:
* A format string `"[Thread {}] [{}] Motors enabled, velocity = {}"` 
* 3 arguments
  * 1 integer - `thread_id`, e.g., `15`
  * 1 string - `component_name`, e.g., `"Motor Controller"`
  * 1 double - `velocity_rpm`, e.g., `0.01`

If this call is made a million times, there are some pieces of information that do not change: format string, the number of arguments, and the type of each argument. These pieces need not be written to a log file again and again, for each call. 

Thus, `binary_log` breaks the logging into 2 files:
* An index file which stores all the static information
* A log file that only stores the dynamic information. 

`binary_log` will thus pack the data in the following format:

* index file
```
<format_string_length> <format_string> <num_args> <arg1_type> ... <argn_type>
<arg1_is_constant> (<arg1_value>)? ... <argn_is_constant> (<argn_value>)? ...
```

* log file
```
<format_string_index> <arg1_value> ... <argn_value>
```

The first byte in the log file is an index into the table in the index file, mapping to a unique chunk of meta information. This is used during the unpacking process to deflate the logs. 

## Packing Integers

`binary_log` packs integers based on value. An argument of type `uint64_t` with a value of `19` will be packed as a `uint8_t`, saving 7 bytes of space (actually 6, one byte is required to store the number of bytes consumed by the integer)

Consider the example:

```cpp
  uint64_t value = 19;
  BINARY_LOG(log, "{}", value);
```

Here is the index file:

```console
foo@bar:~/dev/binary_log$ hexdump -C log.out.index
00000000  02 7b 7d 01 05 00                                 |.{}...|
00000006
```

This index file has 6 bytes of information:
* `0x02` - Length of the format string
* `0x7b 0x7d` - This is the format string `"{}"`
* `0x01` - This is the number of arguments required
* `0x05` - This is the type of the argument `uint64_t` (according to [this](https://github.com/p-ranav/binary_log/blob/master/include/binary_log/detail/args.hpp#L17) enum class)
* `0x00` - To indicate that this value is not a "constant"

Here is the log file

```console
foo@bar:~/dev/binary_log$ hexdump -C log.out
00000000  00 01 13                                          |...|
00000003
```

The log file has 3 bytes: 
* `0x00` indicates that the first format string in the table (in the index file) is being used
* `0x01` indicates that the integer argument takes up 1 byte
* `0x13` is the value - the decimal `19`

## Constants

One can specify a log format argument as a constant by wrapping the value with `binary_log::constant(...)`. When this is detected, the value is stored in the index file instead of the log file as it is now considered "static information" and does not change between calls. 

```cpp
for (auto i = 0; i < 1E9; ++i) {
  BINARY_LOG(log, "Joystick {}: x_min={}, x_max={}, y_min={}, y_max={}",
             binary_log::constant("Nintendo Joycon"),
             binary_log::constant(-0.6),
             binary_log::constant(+0.65),
             binary_log::constant(-0.54),
             binary_log::constant(+0.71));
}
```

The above loop runs in under a second (!!) and writes: (1) 1 GB log file, and (2) 81 byte index file. All of the static information is in the index file. The log file will only contain a 1-byte index (per call) that maps to the meta information in the index file.

```console
foo@bar:~/dev/binary_log$ hexdump -C log.out.index
00000000  1f 4a 6f 79 73 74 69 63  6b 20 7b 7d 3a 20 78 3d  |.Joystick {}: x=|
00000010  7b 7d 2c 20 79 3d 7b 7d  2c 20 7a 3d 7b 7d 2c 20  |{}, y={}, z={}, |
00000020  04 0c 0b 0b 0b 01 0f 4e  69 6e 74 65 6e 64 6f 20  |.......Nintendo |
00000030  4a 6f 79 63 6f 6e 01 9a  99 99 99 99 99 f1 3f 01  |Joycon........?.|
00000040  9a 99 99 99 99 99 01 40  01 66 66 66 66 66 66 0a  |.......@.ffffff.|
00000050  40                                                |@|
00000051
```

# Benchmarks

| Type            | Value                                                                                                     |
| --------------- | --------------------------------------------------------------------------------------------------------- |
| Processor       | 11th Gen Intel(R) Core(TM) i9-11900KF @ 3.50GHz   3.50 GHz                                                |
| Installed RAM   | 32.0 GB (31.9 GB usable)                                                                                  |
| SSD             | [ADATA SX8200PNP](https://www.adata.com/upload/downloadfile/Datasheet_XPG%20SX8200%20Pro_EN_20181017.pdf) |
| OS              | Ubuntu 20.04 LTS running on WSL in Windows 11                                                             |

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

# License

The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
