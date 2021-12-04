#include <chrono>
#include <random>

#include <benchmark/benchmark.h>
#include <binary_log/binary_log.hpp>

static void BM_binary_log_static_string(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // Code to be benchmarked
      BINARY_LOG(log, "Hello World");
    }
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
  remove("log.out.runlength");
}

static void BM_binary_log_constants(benchmark::State& state)
{
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
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
  remove("log.out.runlength");
}

static void BM_binary_log_bool(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", true);
    }
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
  remove("log.out.runlength");
}

static void BM_binary_log_uint8(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", 42);
    }
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
  remove("log.out.runlength");
}

static void BM_binary_log_float(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", 2.71828f);
    }
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
  remove("log.out.runlength");
}

static void BM_binary_log_double(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", 3.14);
    }
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
  remove("log.out.runlength");
}

static void BM_binary_log_string(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    std::string_view str = "Motors enabled!";

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "[Motor Controller] {}", str);
    }
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
  remove("log.out.runlength");
}

static void BM_binary_log_million_integers(benchmark::State& state)
{
  {
    binary_log::binary_log log("log2.out");

    for (auto _ : state) {
      // This code gets timed
      for (int i = 0; i < 1E6; ++i) {
        BINARY_LOG(log, "{}", i);
      }
    }
  }

  state.counters["Logs/s"] =
      benchmark::Counter(1E6 * state.iterations(), benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      1E6 * state.iterations(),
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log2.out");
  remove("log2.out.index");
  remove("log2.out.runlength");
}

BENCHMARK(BM_binary_log_static_string);
BENCHMARK(BM_binary_log_constants);
BENCHMARK(BM_binary_log_bool);
BENCHMARK(BM_binary_log_uint8);
BENCHMARK(BM_binary_log_float);
BENCHMARK(BM_binary_log_double);
BENCHMARK(BM_binary_log_string);
BENCHMARK(BM_binary_log_million_integers)->Unit(benchmark::kMillisecond);

// Run the benchmark
BENCHMARK_MAIN();
