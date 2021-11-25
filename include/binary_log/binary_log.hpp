#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include <moodycamel/concurrentqueue.h>
#define FMT_HEADER_ONLY
#include <sstream>

#include <fmt/format.h>
#include <fmt/os.h>
#include <msgpack.hpp>

class binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;

  std::thread m_formatter_thread;
  moodycamel::ConcurrentQueue<std::function<void()>> m_formatter_queue;
  std::atomic_size_t m_enqueued_for_formatting {0};
  std::mutex m_formatter_mutex;
  std::condition_variable m_formatter_data_ready;

  std::atomic_size_t m_running {true};

  // Format string table
  std::unordered_map<std::string_view, std::size_t> m_format_string_table;
  std::atomic_size_t m_format_string_index {0};

  void formatter_thread_function()
  {
    std::function<void()> format_function;
    while (m_running || m_enqueued_for_formatting > 0) {
      // Wait for the `enqueued` signal
      {
        std::unique_lock<std::mutex> lock {m_formatter_mutex};
        m_formatter_data_ready.wait(
            lock,
            [this] { return m_enqueued_for_formatting > 0 || !m_running; });
      }

      if (m_formatter_queue.try_dequeue(format_function)) {
        format_function();
        m_enqueued_for_formatting -= 1;
      }
    }
  }

public:
  binary_log(std::string_view path)
  {
    // Create the log file
    // All the log contents go here
    m_log_file = fopen(path.data(), "w");
    if (m_log_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }

    // Create the index file
    // All the format_strings go here
    // into a table
    // FORMAT_STRING_1 -> 1
    // FORMAT_STRING_2 -> 2
    // ...
    // FORMAT_STRING_N -> N
    std::string index_file_path = std::string {path} + ".index";
    m_index_file = fopen(index_file_path.c_str(), "w");
    if (m_index_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }

    m_formatter_thread =
        std::thread {&binary_log::formatter_thread_function, this};
  }

  ~binary_log()
  {
    m_running = false;
    m_formatter_thread.join();
  }

  enum class level
  {
    debug,
    info,
    warn,
    error,
    fatal
  };

  long long get_time_since_epoch_in_milliseconds() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
  }

  template<typename... Args>
  void log(const level& level, std::string_view format_string, Args&&... args)
  {
    const auto time_since_epoch_in_milliseconds =
        get_time_since_epoch_in_milliseconds();

    // Schedule write to log file
    m_formatter_queue.enqueue(
        [this,
         time_since_epoch_in_milliseconds,
         level,
         format_string,
         args...]()
        {
          // Update format string table if necessary
          if (m_format_string_table.find(format_string)
              == m_format_string_table.end()) {
            m_format_string_table[format_string] = m_format_string_index;

            // Serialize format string to index file
            std::stringstream os;
            // TODO(pranav): Determine if need to convert format_string into
            // std::string before serializing it
            msgpack::pack(
                os, std::make_pair(format_string, m_format_string_index++));
            const std::string serialized_format_string = os.str();

            // Deserialize immediately for testing
            // msgpack::object_handle oh =
            // msgpack::unpack(serialized_format_string.data(),
            //                                             serialized_format_string.size());
            // auto unpacked = oh.get();
            // auto unpacked_pair = unpacked.as<std::pair<std::string_view,
            // std::size_t>>(); auto unpacked_string = unpacked_pair.first; auto
            // unpacked_index = unpacked_pair.second; fmt::print("{} -> {}\n",
            // unpacked_string, unpacked_index);

            fmt::print(m_index_file, "{}\n", serialized_format_string);
          }
        });

    m_enqueued_for_formatting += 1;
  }
};