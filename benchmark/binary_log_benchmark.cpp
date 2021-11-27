#include <benchmark/benchmark.h>
#include <binary_log/binary_log.hpp>

static void BM_binary_log_latency_static_string(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Starting backup replica garbage collector thread");
  }
}

static void BM_binary_log_latency_one_static_integer(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Backup storage speeds (min): {} MB/s read", 181);
  }
}

static void BM_binary_log_latency_two_static_integers(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log,
               "buffer has consumed {} bytes of extra storage, current "
               "allocation: {} bytes",
               1032024,
               1016544);
  }
}

static void BM_binary_log_latency_one_static_float(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Using tombstone ratio balancer with ratio = {}", 3.1415f);
  }
}

static void BM_binary_log_latency_one_static_double(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Using tombstone ratio balancer with ratio = {}", 0.4);
  }
}

static void BM_binary_log_latency_static_ints_and_doubles(
    benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log,
               "Initialized InfUdDriver buffers: {} receive buffers ({} MB), "
               "{} transmit buffers ({} MB), took {} ms",
               50000,
               97,
               50,
               0,
               26.2);
  }
}

static void BM_binary_log_latency_incrementing_integer(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    for (uint32_t j = 0; j < state.range(0); ++j) {
      BINARY_LOG(log, "Hello logger: msg number {}", j);
    }
  }
}

// Register the function as a benchmark
BENCHMARK(BM_binary_log_latency_static_string);
BENCHMARK(BM_binary_log_latency_one_static_integer);
BENCHMARK(BM_binary_log_latency_two_static_integers);
BENCHMARK(BM_binary_log_latency_one_static_float);
BENCHMARK(BM_binary_log_latency_one_static_double);
BENCHMARK(BM_binary_log_latency_static_ints_and_doubles);
BENCHMARK(BM_binary_log_latency_incrementing_integer)
    ->Args({10000})
    ->Args({100000})
    ->Args({1000000})
    ->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();