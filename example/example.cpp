#include <chrono>
#include <iostream>
#include <random>

#include <binary_log/binary_log.hpp>

static const char* human_size(uint64_t bytes)
{
  const char* suffix[] = {"B", "KB", "MB", "GB", "TB"};
  char length = sizeof(suffix) / sizeof(suffix[0]);

  int i = 0;
  double dblBytes = bytes;

  if (bytes > 1024) {
    for (i = 0; (bytes / 1024) > 0 && i < length - 1; i++, bytes /= 1024)
      dblBytes = bytes / 1024.0;
  }

  static char output[200];
  sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
  return output;
}

template<typename... Ts>
constexpr std::size_t get_arg_total_size(Ts... args)
{
  if constexpr (sizeof...(Ts) == 0) {
    return 0;
  } else {
    return (sizeof(args) + ...);
  }
}

template<typename T>
void run_log_benchmark_dynamic_integer(std::string_view test_name,
                                       std::size_t target_file_size)
{
  binary_log::binary_log log("log.out");

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<T> distr(0, std::numeric_limits<T>::max());

  std::size_t how_many = target_file_size / (1 + sizeof(T));

  auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < how_many; i++) {
    BINARY_LOG(log, "{}", distr(gen));
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  std::size_t estimated_file_size = how_many * (1 + sizeof(T));

  std::cout << test_name << human_size(estimated_file_size) << " ("
            << human_size(estimated_file_size / (duration_ns / 1e9)) << "/s) ["
            << how_many / (duration_ns / 1e9) << " logs/s] "
            << "{Average latency: " << duration_ns / how_many << " ns}"
            << std::endl;
}

int main()
{
  constexpr std::size_t target_file_size = 100 * 1024 * 1024;
  run_log_benchmark_dynamic_integer<uint8_t>("[Random uint8_t  ] ",
                                             target_file_size);
  run_log_benchmark_dynamic_integer<uint16_t>("[Random uint16_t ] ",
                                              target_file_size);
  run_log_benchmark_dynamic_integer<uint32_t>("[Random uint32_t ] ",
                                              target_file_size);
  run_log_benchmark_dynamic_integer<uint64_t>("[Random uint64_t ] ",
                                              target_file_size);
  // run_log_benchmark("[Static string   ]", target_file_size, "Hello world!");
  // run_log_benchmark("[Dynamic integer ]", target_file_size, "Hello logger,
  // msg number: {}", 5); run_log_benchmark("[Dynamic float   ]",
  // target_file_size, "Hello logger, msg number: {}", 3.1415f);
  // run_log_benchmark("[Dynamic double  ]", target_file_size, "Hello logger,
  // msg number: {}", 2.718);

  // binary_log::binary_log log("log.out");

  /*
  // Static string
  BINARY_LOG(log, "Hello world!");

  // Dynamic string
  BINARY_LOG(log, "Hello {}!", "world!");

  // Bool
  BINARY_LOG(log, "Bool: {}", true);

  // Integer types
  BINARY_LOG(log, "Integer: {}", -12);
  BINARY_LOG(log, "Unsigned integer: {}", 34u);
  BINARY_LOG(log, "Long integer: {}", (int64_t)-56);
  BINARY_LOG(log, "Unsigned long integer: {}", (uint64_t)78);

  // Float types
  BINARY_LOG(log, "Float: {}", 123.456f);
  BINARY_LOG(log, "Double: {:.4f}", 78910.1112131415);

  // String types
  BINARY_LOG(log, "String: {}", "Hello world!");

  BINARY_LOG(log, "Const integer: {}", binary_log::constant(5));
  */

  //   for (uint32_t i = 0; i < 1E6; ++i)
  //     BINARY_LOG(log, "Hello logger, msg number: {}", i);
}
