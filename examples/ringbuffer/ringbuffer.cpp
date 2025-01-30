#include <binary_log/binary_log.hpp>

#include <fstream>

template<class log_type, typename T>
void save_data(log_type& log, T&& value)
{
  BINARY_LOG(log, "{}", std::forward<T>(value));
}

int main()
{
  static constexpr size_t log_buffer_size = 100;
  static constexpr size_t index_buffer_size = 1024;
  static constexpr size_t runlength_buffer_size = 1024;
  using Packer = binary_log::ringbuffer_packer<log_buffer_size,
                                                index_buffer_size,
                                                runlength_buffer_size>;
  binary_log::binary_log<Packer> log("log.out");

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

  // ensure run length works
  for (int i = 0; i < 2E1; ++i) {
    BINARY_LOG(log, "Hello logger, msg number: {}", i);
  }
  // for (int i = 0; i < 1E1; ++i) {
  //   save_data(log, i);
  // }

  // now flush the log
  log.flush();

  // Since the ringbuffer only writes to memory, we need to get the string_views
  // into the log/index/runlength buffers and write those to the appropriate
  // file paths
  const Packer &packer = log.get_packer();
  auto log_buffer = packer.get_log_buffer();
  auto index_buffer = packer.get_index_buffer();
  auto runlength_buffer = packer.get_runlength_buffer();

  std::cout << std::format("Log file size: {} bytes\n", log_buffer.size());
  std::cout << std::format("Index file size: {} bytes\n", index_buffer.size());
  std::cout << std::format("Runlength file size: {} bytes\n", runlength_buffer.size());
  std::cout << "--------------------------------\n";
  std::cout << std::format("Total file size: {} bytes\n", log_buffer.size() + index_buffer.size() + runlength_buffer.size());

  auto log_filepath = packer.get_log_path();
  auto index_filepath = packer.get_index_path();
  auto runlength_filepath = packer.get_runlength_path();

  std::ofstream log_file(log_filepath.string(),
                          std::ios::out | std::ios::binary);
  log_file.write((const char*)log_buffer.data(), log_buffer.size());
  log_file.close();

  std::ofstream index_file(index_filepath.string(),
                            std::ios::out | std::ios::binary);
  index_file.write(index_buffer.data(), index_buffer.size());
  index_file.close();

  std::ofstream runlength_file(runlength_filepath.string(),
                               std::ios::out | std::ios::binary);
  runlength_file.write((const char*)runlength_buffer.data(), runlength_buffer.size());
}
