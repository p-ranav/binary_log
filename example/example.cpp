#include <chrono>

#include <binary_log/binary_log.hpp>

int main()
{
  auto start = std::chrono::high_resolution_clock::now();
  {
    binary_log::binary_log log("log.out");
    for (uint32_t i = 0; i < 1E6; ++i) {
      BINARY_LOG(log, "Hello logger, msg number: {}", i);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "Total                : " << diff_ms << " ms" << std::endl;
  std::cout << "Latency              : " << diff_ns / 1E6 << " ns" << std::endl;
  std::cout << "Throughput           : " << 1E6 / (diff_ms / 1000.0) << " msg/s"
            << std::endl;
  std::cout << "Throughput (bytes/s) : "
            << (1E6 * (1 + sizeof(uint32_t))) / (diff_ms / 1000.0) << std::endl;
}
