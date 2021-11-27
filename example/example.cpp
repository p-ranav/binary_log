#include <chrono>
#include <iostream>
#include <random>

#include <binary_log/binary_log.hpp>

void run_static_string_test()
{
  binary_log::binary_log log("test.log");
  constexpr std::size_t how_many = 1E6;
  auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < how_many; ++i) {
    BINARY_LOG(log, "Starting backup replica garbage collector thread");
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "[STATIC STRING          ] Latency: " << diff_ns / how_many
            << " ns\n";
  BINARY_LOG(log, "This is another static string");
}

void run_single_integer_test()
{
  binary_log::binary_log log("test.log");
  constexpr std::size_t how_many = 1E6;
  auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < how_many; ++i) {
    BINARY_LOG(log, "Backup storage speeds (min): {} MB/s read", 181);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "[SINGLE STATIC INTEGER  ] Latency: " << diff_ns / how_many
            << " ns\n";
}

void run_two_integer_test()
{
  binary_log::binary_log log("test.log");
  constexpr std::size_t how_many = 1E6;
  auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < how_many; ++i) {
    BINARY_LOG(log,
               "buffer has consumed {} bytes of extra storage, current "
               "allocation: {} bytes",
               1032024,
               1016544);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "[DOUBLE STATIC INTEGERS ] Latency: " << diff_ns / how_many
            << " ns\n";
}

void run_single_double_test()
{
  binary_log::binary_log log("test.log");
  constexpr std::size_t how_many = 1E6;
  auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < how_many; ++i) {
    BINARY_LOG(log, "Using tombstone ratio balancer with ratio = {}", 0.4);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "[SINGLE STATIC DOUBLE   ] Latency: " << diff_ns / how_many
            << " ns\n";
}

void run_complex_format_test()
{
  binary_log::binary_log log("test.log");
  constexpr std::size_t how_many = 1E6;
  auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < how_many; ++i) {
    BINARY_LOG(log,
               "Initialized InfUdDriver buffers: {} receive buffers ({} MB), "
               "{} transmit buffers ({} MB), took {} ms",
               50000,
               97,
               50,
               0,
               26.2);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "[COMPLEX FORMAT         ] Latency: " << diff_ns / how_many
            << " ns\n";
}

void run_string_concat_test()
{
  binary_log::binary_log log("test.log");
  constexpr std::size_t how_many = 1E6;
  auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < how_many; ++i) {
    BINARY_LOG(
        log,
        "Opened session with coordinator at basic+udp:host={}.{}.{}.{},port={}",
        192,
        168,
        1,
        140,
        12246);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "[STRING CONCAT          ] Latency: " << diff_ns / how_many
            << " ns\n";
}

int main()
{
  run_static_string_test();
  run_single_integer_test();
  run_two_integer_test();
  run_single_double_test();
  run_complex_format_test();
  run_string_concat_test();
}