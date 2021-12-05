#include <cmath>

#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  // This sample exercises the run-length encoding
  // part of binary_log.

  for (std::size_t i = 0; i < 3; ++i) {
    BINARY_LOG(log, "New iteration");

    for (std::size_t j = 0; j < 10; ++j) {
      BINARY_LOG(log, "Integer: {}", j);
    }

    for (std::size_t j = 0; j <= 360; j = j + 36) {
      BINARY_LOG(log, "Float: {:.2f}", sin(j * 3.14 / 180));
    }

    for (std::size_t j = 0; j < 10; ++j) {
      BINARY_LOG(log, "Bool: {}", j % 2 == 0);
    }

    BINARY_LOG(log, "Flushing files");

    for (std::size_t j = 0; j < 3; ++j) {
      auto filename = std::string {"some_file_"} + std::to_string(j);
      BINARY_LOG(log, "Flushed file: {}", filename);
    }

    BINARY_LOG(log, "End of iteration");
  }
}
