#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");
  BINARY_LOG(log, "Hello World");

  for (std::size_t i = 0; i < 5; ++i) {
    BINARY_LOG(log, "binary_log is {}", binary_log::constant("awesome"));
  }

  BINARY_LOG(log, "Dynamic data: {} {}", 123, "abc");

  for (std::size_t i = 0; i < 12345; ++i) {
    BINARY_LOG(log,
               "binary_log performs run-length encoding on some log calls");
  }
}