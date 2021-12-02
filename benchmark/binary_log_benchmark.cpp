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
    BINARY_LOG(log,
               "Backup storage speeds (min): {} MB/s read",
               binary_log::constant(181));
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
               binary_log::constant(1032024),
               binary_log::constant(1016544));
  }
}

static void BM_binary_log_latency_one_static_float(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log,
               "Using tombstone ratio balancer with ratio = {}",
               binary_log::constant(3.1415f));
  }
}

static void BM_binary_log_latency_one_static_double(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log,
               "Using tombstone ratio balancer with ratio = {}",
               binary_log::constant(0.4));
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
               binary_log::constant(50000),
               binary_log::constant(97),
               binary_log::constant(50),
               binary_log::constant(0),
               binary_log::constant(26.2));
  }
}

static void BM_binary_log_latency_static_cstring(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Hello {}", binary_log::constant("World"));
  }
}

static void BM_binary_log_latency_static_std_string(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "Hello {}", binary_log::constant(std::string {"World"}));
  }
}

static void BM_binary_log_latency_static_std_string_view(
    benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(
        log, "Hello {}", binary_log::constant(std::string_view {"World"}));
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

std::string generate_random_string(size_t length)
{
  auto randchar = []() -> char
  {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

static void BM_binary_log_latency_random_std_string_of_size(
    benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "{}", generate_random_string(state.range(0)));
  }
}

// Register the function as a benchmark
BENCHMARK(BM_binary_log_latency_static_string);
BENCHMARK(BM_binary_log_latency_one_static_integer);
BENCHMARK(BM_binary_log_latency_two_static_integers);
BENCHMARK(BM_binary_log_latency_one_static_float);
BENCHMARK(BM_binary_log_latency_one_static_double);
BENCHMARK(BM_binary_log_latency_static_ints_and_doubles);
BENCHMARK(BM_binary_log_latency_static_cstring);
BENCHMARK(BM_binary_log_latency_static_std_string);
BENCHMARK(BM_binary_log_latency_static_std_string_view);
BENCHMARK(BM_binary_log_latency_random_integer);
BENCHMARK(BM_binary_log_latency_random_float);
BENCHMARK(BM_binary_log_latency_random_double);
BENCHMARK(BM_binary_log_latency_random_std_string_of_size)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(32)
    ->Arg(64);

// Run the benchmark
BENCHMARK_MAIN();