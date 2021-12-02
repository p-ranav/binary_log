#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  // Static string
  BINARY_LOG(log, "Hello world!");

  // Dynamic string
  BINARY_LOG(log, "Hello {}!", "world!");

  // Bool
  BINARY_LOG(log, "Bool: {}", true);

  // Integer types
  BINARY_LOG(log, "Integer: {}", -12);
  BINARY_LOG(log, "Unsigned integer: {}", 34u);
  BINARY_LOG(log, "Long integer: {}", (int64_t)-56);
  BINARY_LOG(log, "Unsigned long integer: {}", (uint64_t)78);

  // Float types
  BINARY_LOG(log, "Float: {}", 123.456f);
  BINARY_LOG(log, "Double: {:.4f}", 78910.1112131415);

  // String types
  BINARY_LOG(log, "String: {}", "Hello world!");

  // for (uint32_t i = 0; i < 1E6; ++i)
  //   BINARY_LOG(log, "Hello logger, msg number: {}", i);
}