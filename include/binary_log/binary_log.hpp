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

#include <fmt/args.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <msgpack.hpp>
#include <msgpack/fbuffer.hpp>

struct binary_log_message
{
  uint8_t log_level;
  std::size_t format_string_index;
  std::vector<std::string> format_string_args;
  MSGPACK_DEFINE(log_level, format_string_index, format_string_args);
};

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
        m_enqueued_for_formatting--;
      }
    }
  }

  void formatter_thread_function_bulk()
  {
    constexpr std::size_t bulk_dequeue_size = 32;
    std::function<void()> format_functions[bulk_dequeue_size];
    while (m_running || m_enqueued_for_formatting > 0) {
      // Wait for the `enqueued` signal
      {
        std::unique_lock<std::mutex> lock {m_formatter_mutex};
        m_formatter_data_ready.wait(lock,
                                    [this] {
                                      return m_enqueued_for_formatting
                                          >= bulk_dequeue_size
                                          || !m_running;
                                    });
      }

      if (m_formatter_queue.try_dequeue_bulk(format_functions,
                                             bulk_dequeue_size)) {
        for (std::size_t i = 0; i < bulk_dequeue_size; i++) {
          if (format_functions[i]) {
            format_functions[i]();
            m_enqueued_for_formatting--;
          }
        }
      }
    }
  }

  template<class Tuple>
  std::vector<std::string> to_vector_internal(Tuple&& tuple)
  {
    return std::apply(
        [](auto&&... elems)
        {
          return std::vector<std::string> {
              fmt::to_string(std::forward<decltype(elems)>(elems))...};
        },
        std::forward<Tuple>(tuple));
  }

  template<typename... Args>
  std::vector<std::string> to_vector(Args&&... args)
  {
    return to_vector_internal(
        std::tuple<Args...> {std::forward<Args>(args)...});
  }

public:
  binary_log(std::string_view path)
  {
    // Create the log file
    // All the log contents go here
    m_log_file = fopen(path.data(), "wb");
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
    m_index_file = fopen(index_file_path.c_str(), "wb");
    if (m_index_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }

    m_formatter_thread =
        std::thread {&binary_log::formatter_thread_function_bulk, this};
  }

  ~binary_log()
  {
    m_running = false;
    m_formatter_data_ready.notify_all();
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

  template<typename... Args>
  void log(const level& level, std::string_view format_string, Args&&... args)
  {
    // Schedule write to log file
    m_formatter_queue.enqueue(
        [this, level, format_string, args...]()
        {
          // Update format string table if necessary
          if (m_format_string_table.find(format_string)
              == m_format_string_table.end()) {
            m_format_string_table[format_string] = m_format_string_index++;

            msgpack::fbuffer os(m_index_file);
            msgpack::pack(os, format_string);
            msgpack::pack(os, "\n");
          }

          // Serialize log message
          binary_log_message msg;
          msg.log_level = static_cast<uint8_t>(level);
          msg.format_string_index = m_format_string_table[format_string];
          msg.format_string_args = std::move(to_vector(args...));

          msgpack::fbuffer os(m_log_file);
          msgpack::pack(os, msg);
        });

    m_enqueued_for_formatting += 1;
  }
};