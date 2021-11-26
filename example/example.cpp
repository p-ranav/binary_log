#include <binary_log/binary_log.hpp>

int main()
{
  binary_log log("test.log");
  constexpr std::size_t how_many = 1E6;
  for (std::size_t j = 0; j < how_many; ++j) {
    LOG_INFO(log, "Hello logger: msg number {}", j);
  }
}