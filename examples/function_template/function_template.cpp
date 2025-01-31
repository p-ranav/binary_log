#include <binary_log/binary_log.hpp>

template<class log_type, typename T>
void save_data(log_type& log, T&& value)
{
  BINARY_LOG(log, "{}", std::forward<T>(value));
}

int main()
{
  binary_log::binary_log<> log("log.out");

  save_data(log, 5);
  save_data(log, 3.14);
  save_data(log, binary_log::constant(2*3.14159265358));
  save_data(log, "Hello, world!");
  save_data(log, std::string("This is a string"));
  auto complex_string = std::format("This is a complex string with a number: {}", 42);
  save_data(log, complex_string);
  BINARY_LOG(log, "This is a string with a different number: {}", (uint8_t)43);
  save_data(log, true);
  save_data(log, 2.7182818284590452353602874713527f);
  save_data(log, 'a');

  for (int i = 0; i < 1E1; ++i) {
    save_data(log, i);
    BINARY_LOG(log, "Hello logger, msg number: {}", i);
  }

}
