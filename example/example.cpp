#include <chrono>
#include <iostream>

#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  constexpr auto how_many = 1E6;
  auto start = std::chrono::high_resolution_clock::now();

  for (auto i = 0; i < how_many; ++i)
    BINARY_LOG(log, "Hello World");

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end
                                                                     - start)
                   .count()
            << " us\n";
}
