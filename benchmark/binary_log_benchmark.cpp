#include <chrono>
#include <random>

#include <benchmark/benchmark.h>
#include <binary_log/binary_log.hpp>

template<typename T>
static void BM_binary_log_static_integer(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", T(state.range(0)));
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

static void BM_binary_log_static_float(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", 2.7182818284590452353602874713527f);
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

static void BM_binary_log_static_double(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", 3.1415926535897932384626433832795);
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

static void BM_binary_log_static_string(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "Hello {}", "World");
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

template<typename T>
static void BM_binary_log_random_integer(benchmark::State& state)
{
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<T> distr(std::numeric_limits<T>::min(),
                                         std::numeric_limits<T>::max());

  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", distr(rng));
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

template<typename T>
static void BM_binary_log_random_real(benchmark::State& state)
{
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<T> distr(1, 1E6);

  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", distr(rng));
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

static void BM_binary_log_billion_integers(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      for (int i = 0; i < 1E9; ++i)
	BINARY_LOG(log, "Hello logger, msg number: {}", i);
    }
  }

  state.counters["Logs/s"] =
      benchmark::Counter(state.iterations() * 1E9, benchmark::Counter::kIsRate);

  state.counters["Latency"] = benchmark::Counter(
      state.iterations() * 1E9,
      benchmark::Counter::kIsRate | benchmark::Counter::kInvert);

  remove("log.out");
  remove("log.out.index");
  remove("log.out.runlength");
}

BENCHMARK_TEMPLATE(BM_binary_log_static_integer, uint8_t)->Arg(42);
BENCHMARK_TEMPLATE(BM_binary_log_static_integer, uint16_t)->Arg(395);
BENCHMARK_TEMPLATE(BM_binary_log_static_integer, uint32_t)->Arg(3123456789);
BENCHMARK_TEMPLATE(BM_binary_log_static_integer, uint64_t)
    ->Arg(9876543123456789);
BENCHMARK_TEMPLATE(BM_binary_log_static_integer, int8_t)->Arg(-42);
BENCHMARK_TEMPLATE(BM_binary_log_static_integer, int16_t)->Arg(-395);
BENCHMARK_TEMPLATE(BM_binary_log_static_integer, int32_t)->Arg(-123456789);
BENCHMARK_TEMPLATE(BM_binary_log_static_integer, int64_t)
    ->Arg(-9876543123456789);
BENCHMARK(BM_binary_log_static_float);
BENCHMARK(BM_binary_log_static_double);
BENCHMARK(BM_binary_log_static_string);
BENCHMARK_TEMPLATE(BM_binary_log_random_integer, uint8_t);
BENCHMARK_TEMPLATE(BM_binary_log_random_integer, uint16_t);
BENCHMARK_TEMPLATE(BM_binary_log_random_integer, uint32_t);
BENCHMARK_TEMPLATE(BM_binary_log_random_integer, uint64_t);
BENCHMARK_TEMPLATE(BM_binary_log_random_integer, int8_t);
BENCHMARK_TEMPLATE(BM_binary_log_random_integer, int16_t);
BENCHMARK_TEMPLATE(BM_binary_log_random_integer, int32_t);
BENCHMARK_TEMPLATE(BM_binary_log_random_integer, int64_t);
BENCHMARK_TEMPLATE(BM_binary_log_random_real, float);
BENCHMARK_TEMPLATE(BM_binary_log_random_real, double);
BENCHMARK(BM_binary_log_billion_integers);

// Run the benchmark
BENCHMARK_MAIN();
