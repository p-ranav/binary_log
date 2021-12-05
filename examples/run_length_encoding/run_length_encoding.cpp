#include <cmath>

#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  for (std::size_t i = 0; i < 1; ++i) {
    for (std::size_t j = 0; j < 10; ++j) {
      BINARY_LOG(log, "Integer: {}", j);
    }

    for (std::size_t j = 0; j <= 360; j = j + 45) {
      BINARY_LOG(log, "Float: {:.2f}", sin(j * 3.14 / 180));
    }

    for (std::size_t j = 0; j < 10; ++j) {
      BINARY_LOG(log, "Bool: {}", j % 2 == 0);
    }
  }
}
