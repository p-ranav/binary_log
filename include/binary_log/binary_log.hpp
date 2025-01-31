#pragma once
#include <filesystem>
#include <iostream>
#include <string>

#include <binary_log/constant.hpp>
#include <binary_log/detail/args.hpp>
#include <binary_log/detail/packer.hpp>
#include <binary_log/detail/ringbuffer_packer.hpp>

namespace binary_log
{
template<class Packer = packer<1 * 1024 * 1024, 32, 32>>
class binary_log
{
  Packer m_packer;
  uint8_t m_format_string_index {0};

public:
  binary_log(const char* path)
      : m_packer(path)
  {
  }

  binary_log(const std::filesystem::path& path)
      : m_packer(path)
  {
  }

  const Packer& get_packer() const
  {
    return m_packer;
  }

  ~binary_log()
  {
    m_packer.flush();
  }

  void flush()
  {
    m_packer.flush();
  }

  template<const char* format_string, class... Args>
  constexpr inline uint8_t log_index(Args&&... args)
  {
    // SPEC:
    // <format-string-length> <format-string>
    // <number-of-arguments> <arg-type-1> <arg-type-2> ... <arg-type-N>
    // <arg-1-is- const> <arg-1-value>? <arg-2-is- const> <arg-2-value>?
    // ...
    //
    // If the arg is not an lvalue, it is stored in the index file
    // and the value is not stored in the log file
    constexpr uint8_t num_args = sizeof...(Args);

    m_format_string_index++;

    // Write the length of the format string
    m_packer. template write_format_string_to_index_file<format_string>();

    // Write the number of args taken by the format string
    m_packer.write_num_args_to_index_file(num_args);

    // Write the type of each argument
    if constexpr (num_args > 0) {
      m_packer.update_index_file(std::forward<Args>(args)...);
    }

    return m_format_string_index - 1;
  }

  template<class... Args>
  inline void log(std::size_t pos, Args&&... args)
  {
    constexpr auto num_args = sizeof...(Args);

    // Write to the main log file
    // SPEC:
    // <format-string-index> <arg1> <arg2> ... <argN>
    // where:
    //   <format-string-index> is the index of the format string in the index
    //   file <arg1> <arg2> ... <argN> are the arguments to the format string
    //     Each <arg> is a pair: <type, value>
    uint16_t current_index = static_cast<uint16_t>(pos);
    m_packer.pack_format_string_index(current_index);

    // Write the args
    if constexpr (num_args > 0) {
      if constexpr (!all_args_are_constants<Args...>()) {
        m_packer.update_log_file(std::forward<Args>(args)...);
      }
    }
  }
};

}  // namespace binary_log

#define BINARY_LOG_CONCAT0(a, b) a##b
#define BINARY_LOG_CONCAT(a, b) BINARY_LOG_CONCAT0(a, b)

#define BINARY_LOG(logger, format_string, ...) \
  { \
    constexpr static char BINARY_LOG_CONCAT(__binary_log_format_string, \
                                            __LINE__)[] = format_string; \
    static std::size_t BINARY_LOG_CONCAT(__binary_log_format_string_id_pos, \
                                         __LINE__) = \
        logger.template log_index<BINARY_LOG_CONCAT(__binary_log_format_string, \
                                           __LINE__)>(__VA_ARGS__); \
    logger.template log(BINARY_LOG_CONCAT(__binary_log_format_string_id_pos, __LINE__), \
               ##__VA_ARGS__); \
  }
