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
  save_data(log, "Hello, world!");
  save_data(log, std::string("This is a string"));
  save_data(log, true);
  save_data(log, 2.7182818284590452353602874713527f);
  save_data(log, 'a');
}
