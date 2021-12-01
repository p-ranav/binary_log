#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  for (uint32_t i = 0; i < 40E6; ++i)
    BINARY_LOG(log, "Hello logger, msg number: {}", i);
}
