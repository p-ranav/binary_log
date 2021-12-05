#include <chrono>
#include <random>

#include <benchmark/benchmark.h>
#include <binary_log/binary_log.hpp>

static void BM_binary_log_random_bool(benchmark::State& state)
{
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<uint8_t> distr(0, 1);
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", bool(distr(rng)));
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

std::string generate_random_string(const int len)
{
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

template<std::size_t N>
static void BM_binary_log_random_string(benchmark::State& state)
{
  {
    binary_log::binary_log log("log.out");

    for (auto _ : state) {
      // This code gets timed
      BINARY_LOG(log, "{}", generate_random_string(N));
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

BENCHMARK(BM_binary_log_random_bool);
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
BENCHMARK_TEMPLATE(BM_binary_log_random_string, 2);
BENCHMARK_TEMPLATE(BM_binary_log_random_string, 4);
BENCHMARK_TEMPLATE(BM_binary_log_random_string, 8);
BENCHMARK_TEMPLATE(BM_binary_log_random_string, 16);
BENCHMARK_TEMPLATE(BM_binary_log_random_string, 32);
BENCHMARK_TEMPLATE(BM_binary_log_random_string, 64);

// Run the benchmark
BENCHMARK_MAIN();
