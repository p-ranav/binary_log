#pragma once
#include <iostream>
#include <string>

#include <binary_log/constant.hpp>
#include <binary_log/detail/args.hpp>
#include <binary_log/detail/packer.hpp>

namespace binary_log
{
template<std::size_t buffer_size = 1024 * 1024,
         typename format_string_index_type = uint8_t>
class binary_log
{
  packer<buffer_size, format_string_index_type> m_packer;
  format_string_index_type m_format_string_index {0};

public:
  binary_log(const char* path)
      : m_packer(path)
  {
  }

  void flush()
  {
    m_packer.flush();
  }

  template<class... Args>
  inline uint8_t log_index(std::string_view f, Args&&... args)
  {
    // SPEC:
    // <format-string-length> <format-string>
    // <number-of-arguments> <arg-type-1> <arg-type-2> ... <arg-type-N>
    // <arg-1-is- const> <arg-1-value>? <arg-2-is- const> <arg-2-value>?
    // ...
    //
    // If the arg is not an lvalue, it is stored in the index file
    // and the value is not stored in the log file
    const uint8_t num_args = sizeof...(Args);

    m_format_string_index++;

    // Write the length of the format string
    m_packer.write_format_string_to_index_file(f);

    // Write the number of args taken by the format string
    m_packer.write_num_args_to_index_file(num_args);

    // Write the type of each argument
    if (num_args > 0) {
      m_packer.update_index_file(std::forward<Args>(args)...);
    }

    return m_format_string_index - 1;
  }

  template<class... Args>
  inline void log(std::size_t pos, Args&&... args)
  {
    const uint8_t num_args = sizeof...(Args);

    // Write to the main log file
    // SPEC:
    // <format-string-index> <arg1> <arg2> ... <argN>
    // where:
    //   <format-string-index> is the index of the format string in the index
    //   file <arg1> <arg2> ... <argN> are the arguments to the format string
    //     Each <arg> is a pair: <type, value>
    format_string_index_type current_index =
        static_cast<format_string_index_type>(pos);
    m_packer.pack_format_string_index(current_index);

    // Write the args
    if (num_args > 0 && !all_args_are_constants(std::forward<Args>(args)...)) {
      m_packer.update_log_file(std::forward<Args>(args)...);
    }
  }
};

}  // namespace binary_log

#define CONCAT0(a, b) a##b
#define CONCAT(a, b) CONCAT0(a, b)

#define BINARY_LOG(logger, format_string, ...) \
  { \
    static std::size_t CONCAT(__binary_log_format_string_id_pos, __LINE__) = \
        logger.log_index(format_string, ##__VA_ARGS__); \
    logger.log(CONCAT(__binary_log_format_string_id_pos, __LINE__), \
               ##__VA_ARGS__); \
  }
