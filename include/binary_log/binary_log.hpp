#pragma once
#include <iostream>
#include <string>

#include <binary_log/detail/args.hpp>
#include <binary_log/constant.hpp>
#include <binary_log/detail/packer.hpp>

namespace binary_log
{
template<typename format_string_index_type = uint8_t>
class binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;
  format_string_index_type m_format_string_index {0};

public:
  binary_log(const char* path)
  {
    // Create the log file
    // All the log contents go here
    m_log_file = fopen(path, "wb");
    if (m_log_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }

    // Create the index file
    std::string index_file_path = std::string {path} + ".index";
    m_index_file = fopen(index_file_path.data(), "wb");
    if (m_index_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }
  }

  ~binary_log() noexcept
  {
    fclose(m_log_file);
    fclose(m_index_file);
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
    const uint8_t format_string_length = f.size();
    fwrite(&format_string_length, 1, 1, m_index_file);

    // Write the format string
    fwrite(f.data(), 1, format_string_length, m_index_file);

    // Write the number of args taken by the format string
    fwrite(&num_args, sizeof(uint8_t), 1, m_index_file);

    // Write the type of each argument
    if (num_args > 0) {
      packer::update_index_file(m_index_file, std::forward<Args>(args)...);
    }

    return m_format_string_index - 1;
  }

  template<class... Args>
  inline void log(uint8_t pos, Args&&... args)
  {
    const uint8_t num_args = sizeof...(Args);

    // Write to the main log file
    // SPEC:
    // <format-string-index> <arg1> <arg2> ... <argN>
    // where:
    //   <format-string-index> is the index of the format string in the index
    //   file <arg1> <arg2> ... <argN> are the arguments to the format string
    //     Each <arg> is a pair: <type, value>

    fwrite(&pos, sizeof(uint8_t), 1, m_log_file);

    // Write the args
    if (num_args > 0 && !all_args_are_constants(std::forward<Args>(args)...)) {
      packer::update_log_file(m_log_file, std::forward<Args>(args)...);
    }
  }
};

}  // namespace binary_log

#define CONCAT0(a, b) a##b
#define CONCAT(a, b) CONCAT0(a, b)

#define BINARY_LOG(logger, format_string, ...) \
  { \
    static uint8_t CONCAT(__binary_log_format_string_id_pos, __LINE__) = \
        logger.log_index(format_string, ##__VA_ARGS__); \
    logger.log(CONCAT(__binary_log_format_string_id_pos, __LINE__), \
               ##__VA_ARGS__); \
  }
