#include <binary_log/binary_log.hpp>

auto main() -> int
{
  binary_log log("test.log");

  log.log(binary_log::level::info, "Hello, world!");
  log.log(binary_log::level::warn, "Hello, {}!", "world");

  return 0;
}
