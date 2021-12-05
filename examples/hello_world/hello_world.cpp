#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");
  BINARY_LOG(log, "Hello, world!");
}
