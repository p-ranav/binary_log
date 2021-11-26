#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

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
  std::map<std::string_view, std::size_t> m_format_string_table;
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
            if (m_enqueued_for_formatting > 0) {
              m_enqueued_for_formatting--;
            }
          }
        }
      }
    }
  }

  enum class fmt_arg_type : uint8_t
  {
    type_size_t,
    type_char,
    type_int,
    type_float,
    type_double
  };

  constexpr static inline uint8_t string_length(const char* str)
  {
    return *str ? 1 + string_length(str + 1) : 0;
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
            constexpr uint8_t log_level_byte = \
                static_cast<uint8_t>(log_level); \
            fwrite(&log_level_byte, 1, 1, logger.m_index_file); \
\
            constexpr uint8_t format_string_length = \
                binary_log::string_length(format_string); \
            fwrite(&format_string_length, 1, 1, logger.m_index_file); \
\
            fwrite( \
                format_string, 1, format_string_length, logger.m_index_file); \
\
            constexpr uint8_t num_args = sizeof...(vargs); \
            fwrite(&num_args, 1, 1, logger.m_index_file); \
\
            logger.m_format_string_index += 1; \
          } \
        }); \
    logger.m_enqueued_for_formatting += 1; \
    logger.m_formatter_data_ready.notify_one(); \
  } \
  (__VA_ARGS__);

#define LOG_DEBUG(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::debug, format_string, __VA_ARGS__)

#define LOG_INFO(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::info, format_string, __VA_ARGS__)

#define LOG_WARN(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::warn, format_string, __VA_ARGS__)

#define LOG_ERROR(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::error, format_string, __VA_ARGS__)

#define LOG_FATAL(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::fatal, format_string, __VA_ARGS__)
