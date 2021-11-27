#include <random>

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

static void BM_binary_log_latency_random_integer(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> distr(0, 1E6);

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Hello logger: msg number {}", distr(gen));
  }
}

static void BM_binary_log_latency_random_float(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  double lower_bound = 0;
  double upper_bound = 1E6;
  std::uniform_real_distribution<float> distr(lower_bound, upper_bound);
  std::default_random_engine gen;

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Hello logger: msg number {}", distr(gen));
  }
}

static void BM_binary_log_latency_random_double(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  double lower_bound = 0;
  double upper_bound = 1E6;
  std::uniform_real_distribution<double> distr(lower_bound, upper_bound);
  std::default_random_engine gen;

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Hello logger: msg number {}", distr(gen));
  }
}

// Register the function as a benchmark
BENCHMARK(BM_binary_log_latency_static_string);
BENCHMARK(BM_binary_log_latency_one_static_integer);
BENCHMARK(BM_binary_log_latency_two_static_integers);
BENCHMARK(BM_binary_log_latency_one_static_float);
BENCHMARK(BM_binary_log_latency_one_static_double);
BENCHMARK(BM_binary_log_latency_static_ints_and_doubles);
BENCHMARK(BM_binary_log_latency_random_integer);
BENCHMARK(BM_binary_log_latency_random_float);
BENCHMARK(BM_binary_log_latency_random_double);

// Run the benchmark
BENCHMARK_MAIN();