#include <chrono>

#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  auto start = std::chrono::high_resolution_clock::now();
  for (uint64_t i = 0; i < 1E6; ++i) {
    BINARY_LOG(log, "Hello logger, msg number: {}", i);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "Time    : " << diff_ns << " ns" << std::endl;
  std::cout << "Latency : " << diff_ns / 1E9 << " ns" << std::endl;
}
