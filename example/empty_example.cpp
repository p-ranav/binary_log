#include <binary_log/binary_log.hpp>

int main()
{
  binary_log log("test.log");

  std::vector<std::thread> threads;

  constexpr std::size_t total_messages = 1E6;
  constexpr std::size_t num_threads = 1;
  constexpr std::size_t num_messages_per_thread = total_messages / num_threads;

  for (std::size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back(
        [&log]()
        {
          for (std::size_t j = 0; j < num_messages_per_thread; ++j) {
            log.log(binary_log::level::info, "Hello logger: msg number {}", j);
          }
        });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // for (int i = 0; i < 1E6; ++i) {
  //   log.log(binary_log::level::info, "Hello logger: msg number {}", i);
  // }
}
