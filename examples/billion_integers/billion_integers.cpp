#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  for (int i = 0; i < 1E9; ++i) {
    BINARY_LOG(log, "Hello logger, msg number: {}", i);
  }
}
