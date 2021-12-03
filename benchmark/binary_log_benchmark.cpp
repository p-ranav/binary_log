#include <chrono>
#include <random>

#include <benchmark/benchmark.h>
#include <binary_log/binary_log.hpp>

static void BM_binary_log_static_string(benchmark::State& state)
{
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // Code to be benchmarked
    BINARY_LOG(log, "Hello World");
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  // 1 byte to store the format string index
  state.counters["Bytes/s"] = benchmark::Counter(
      state.iterations() * sizeof(uint8_t), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
}

static void BM_binary_log_constants(benchmark::State& state)
{
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // Code to be benchmarked
    BINARY_LOG(log,
               "Integer: {}, Float: {}, Double: {}, String: {}",
               binary_log::constant(42),
               binary_log::constant(3.14f),
               binary_log::constant(2.718),
               binary_log::constant("Hello"));
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  // 1 byte to store the format string index
  state.counters["Bytes/s"] = benchmark::Counter(
      state.iterations() * sizeof(uint8_t), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
}

static void BM_binary_log_integer(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "{}", 42);
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  // 42 can be stored as a uint8_t, so binary_log will just write 1 byte
  const auto size_of_each_log_entry = (1 + 1 + sizeof(uint8_t));
  state.counters["Bytes/s"] = benchmark::Counter(
      state.iterations() * size_of_each_log_entry, benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
}

static void BM_binary_log_double(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "{}", 3.14);
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  const auto size_of_each_log_entry = (1 + sizeof(double));
  state.counters["Bytes/s"] = benchmark::Counter(
      state.iterations() * size_of_each_log_entry, benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
}

static void BM_binary_log_string(benchmark::State& state)
{
  // Perform setup here
  binary_log::binary_log log("log.out");

  std::string_view str = "Motors enabled!";

  for (auto _ : state) {
    // This code gets timed
    BINARY_LOG(log, "[Motor Controller] {}", str);
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  const auto size_of_each_log_entry = (1 + 1 + str.size());
  state.counters["Bytes/s"] = benchmark::Counter(
      state.iterations() * size_of_each_log_entry, benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
}

BENCHMARK(BM_binary_log_static_string);
BENCHMARK(BM_binary_log_constants);
BENCHMARK(BM_binary_log_integer);
BENCHMARK(BM_binary_log_double);
BENCHMARK(BM_binary_log_string);

// Run the benchmark
BENCHMARK_MAIN();
