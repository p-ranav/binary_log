# binary_log

This is the binary_log project.

## Benchmarks

```console
foo@bar:~/dev/binary_log$ ./build/benchmark/binary_log_benchmark
2021-11-28T14:13:08-06:00
Running ./build/benchmark/binary_log_benchmark
Run on (8 X 2611 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
---------------------------------------------------------------------------------------------
Benchmark                                                   Time             CPU   Iterations
---------------------------------------------------------------------------------------------
BM_binary_log_latency_static_string                      9.06 ns         9.00 ns     74666667
BM_binary_log_latency_one_static_integer                 9.51 ns         9.63 ns     74666667
BM_binary_log_latency_two_static_integers                9.32 ns         9.21 ns     74666667
BM_binary_log_latency_one_static_float                   9.37 ns         9.42 ns     74666667
BM_binary_log_latency_one_static_double                  9.14 ns         9.24 ns     89600000
BM_binary_log_latency_static_ints_and_doubles            9.34 ns         9.42 ns     74666667
BM_binary_log_latency_static_cstring                     8.96 ns         9.03 ns     64000000
BM_binary_log_latency_static_std_string                  11.7 ns         11.7 ns     64000000
BM_binary_log_latency_static_std_string_view             9.51 ns         9.42 ns     74666667
BM_binary_log_latency_random_integer                     29.4 ns         29.3 ns     22400000
BM_binary_log_latency_random_float                       27.3 ns         26.9 ns     32000000
BM_binary_log_latency_random_double                      32.3 ns         32.2 ns     21333333
BM_binary_log_latency_random_std_string_of_size/4        60.3 ns         60.0 ns     11200000
BM_binary_log_latency_random_std_string_of_size/8        85.5 ns         85.4 ns      8960000
BM_binary_log_latency_random_std_string_of_size/16        154 ns          153 ns      4480000
BM_binary_log_latency_random_std_string_of_size/32        269 ns          267 ns      2635294
BM_binary_log_latency_random_std_string_of_size/64        497 ns          500 ns      1000000
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
