#pragma once
#include <map>
#include <string>
#include <string_view>

#include <binary_log/packer.hpp>

namespace binary_log
{
struct binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;

  // Format string table
  std::map<std::string_view, std::size_t> m_format_string_table;
  std::size_t m_format_string_index {0};

  template<typename T>
  void pack_arg(const T& input)
  {
    packer::pack(m_log_file, input);
  }

  template<class T, class... Ts>
  void pack_args(T&& first, Ts&&... rest)
  {
    pack_arg(std::forward<T>(first));

    if constexpr (sizeof...(rest) > 0) {
      pack_args(std::forward<Ts>(rest)...);
    }
  }

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
  }
};

}  // namespace binary_log

#define BINARY_LOG(logger, format_string, ...) \
  [&logger]<typename... Args>(Args && ... args) \
  { \
    if (logger.m_format_string_table.find(format_string) \
        == logger.m_format_string_table.end()) \
    { \
      logger.m_format_string_table[format_string] = \
          logger.m_format_string_index++; \
\
      /* Write the length of the format string */ \
      constexpr uint8_t format_string_length = \
          binary_log::binary_log::string_length(format_string); \
      fwrite(&format_string_length, 1, 1, logger.m_index_file); \
\
      /* Write the format string */ \
      fwrite(format_string, 1, format_string_length, logger.m_index_file); \
\
      /* Write the number of args taken by the format string */ \
      constexpr uint8_t num_args = sizeof...(args); \
      fwrite(&num_args, 1, 1, logger.m_index_file); \
    } \
\
    /* Write the format string index */ \
    uint8_t format_string_index = logger.m_format_string_table[format_string]; \
    fwrite(&format_string_index, 1, 1, logger.m_log_file); \
\
    /* Write the args */ \
    if constexpr (sizeof...(args) > 0) { \
      logger.pack_args(std::forward<Args>(args)...); \
    } \
  } \
  (__VA_ARGS__);
