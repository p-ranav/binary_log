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

struct binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;

  struct MyTraits : public moodycamel::ConcurrentQueueDefaultTraits
  {
    static const size_t BLOCK_SIZE = 256;  // Use bigger blocks
  };

  std::thread m_formatter_thread;
  moodycamel::ConcurrentQueue<std::function<void()>, MyTraits>
      m_formatter_queue;
  std::atomic_size_t m_enqueued_for_formatting {0};
  std::mutex m_formatter_mutex;
  std::condition_variable m_formatter_data_ready;

  std::atomic_size_t m_running {true};

  // Format string table
  std::unordered_map<std::string_view, std::size_t> m_format_string_table;
  std::atomic_size_t m_format_string_index {0};

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

  enum class fmt_arg_type
  {
    type_size_t
  };

  // TODO(pranav): Add overloads of this function for all supported fmt arg
  // types
  constexpr static inline uint8_t get_arg_type(std::size_t)
  {
    return static_cast<uint8_t>(fmt_arg_type::type_size_t);
  }

  template<typename T>
  constexpr static inline void pack_arg(msgpack::fbuffer& os, T&& arg)
  {
    msgpack::pack(os, get_arg_type(arg));
    msgpack::pack(os, arg);
  }

  template<class T, class... Ts>
  constexpr static inline void pack_args(msgpack::fbuffer& os,
                                         T const& first,
                                         Ts const&... rest)
  {
    pack_arg(os, first);

    if constexpr (sizeof...(rest) > 0) {
      pack_args(os, rest...);
    }
  }

  static inline constexpr std::size_t length(const char* str)
  {
    return *str ? 1 + length(str + 1) : 0;
  }

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
};

#define BINARY_LOG(logger, log_level, format_string, ...) \
  [&logger]<typename... Args>(Args && ... args) \
  { \
    logger.m_formatter_queue.enqueue( \
        [&logger, ... vargs = std::forward<Args>(args)]() mutable \
        { \
          if (logger.m_format_string_table.find(format_string) \
              == logger.m_format_string_table.end()) \
          { \
            logger.m_format_string_table[format_string] = \
                logger.m_format_string_index++; \
\
            msgpack::fbuffer os(logger.m_index_file); \
            msgpack::pack(os, static_cast<uint8_t>(log_level)); \
            msgpack::pack(os, logger.m_format_string_index.load()); \
            constexpr size_t format_string_length = \
                binary_log::length(format_string); \
            msgpack::pack(os, format_string_length); \
            msgpack::pack(os, format_string); \
\
            logger.m_format_string_index += 1; \
          } \
\
          msgpack::fbuffer os(logger.m_log_file); \
          msgpack::pack(os, logger.m_format_string_table[format_string]); \
          msgpack::pack(os, sizeof...(vargs)); \
          binary_log::pack_args(os, vargs...); \
        }); \
  } \
  (__VA_ARGS__); \
  logger.m_enqueued_for_formatting += 1; \
  logger.m_formatter_data_ready.notify_one();

#define LOG_INFO(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::info, format_string, __VA_ARGS__)
