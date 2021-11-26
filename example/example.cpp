#include <chrono>
#include <iostream>

#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("test.log");

  constexpr uint64_t how_many = 1E6;
  auto start = std::chrono::high_resolution_clock::now();
  for (uint64_t j = 0; j < how_many; ++j) {
    LOG_INFO(log, "{}", j);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto diff_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  std::cout << "Latency: " << diff_ns / how_many << " ns" << std::endl;
  std::cout << "Total: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end
                                                                     - start)
                   .count()
            << " ms" << std::endl;
}