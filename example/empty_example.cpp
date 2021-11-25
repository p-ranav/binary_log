#include <binary_log/binary_log.hpp>

auto main() -> int
{
  binary_log log("test.log");

  for (int i = 0; i < 1E6; ++i) {
    log.log(binary_log::level::info, "Hello logger: msg number {}", i);
  }

  return 0;
}
