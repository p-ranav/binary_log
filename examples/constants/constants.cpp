#include <binary_log/binary_log.hpp>

int main()
{
  binary_log::binary_log log("log.out");

  for (auto i = 0; i < 1E9; ++i) {
    BINARY_LOG(log,
               "Joystick {}: x_min={}, x_max={}, y_min={}, y_max={}",
               binary_log::constant("Nintendo Joycon"),
               binary_log::constant(-0.6),
               binary_log::constant(+0.65),
               binary_log::constant(-0.54),
               binary_log::constant(+0.71));
  }
}
