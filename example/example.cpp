#include <chrono>
#include <iostream>

#include <binary_log/binary_log.hpp>

int main()
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
  std::cout << "Latency: " << diff_ns / how_many << " ns\n";
  std::cout << "Total: " << diff_ns << " ns\n";
}