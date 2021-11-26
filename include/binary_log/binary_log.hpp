#pragma once
#include <map>
#include <string>
#include <string_view>

#include <binary_log/packer.hpp>

namespace binary_log
{
template<unsigned N>
struct fixed_string
{
  char buf[N + 1] {};
  constexpr fixed_string(char const* s)
  {
    for (unsigned i = 0; i != N; ++i)
      buf[i] = s[i];
  }
  constexpr operator char const *() const
  {
    return buf;
  }
};
template<unsigned N>
fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

struct binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;

  // Format string table
  std::map<std::string_view, std::size_t> m_format_string_table;
  std::size_t m_format_string_index {0};  // 0 means "{}"

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

  constexpr static inline bool strings_equal(char const* a, char const* b)
  {
    return *a == *b && (*a == '\0' || strings_equal(a + 1, b + 1));
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

  ~binary_log() noexcept
  {
    fclose(m_log_file);
    fclose(m_index_file);
  }

  template<fixed_string F, class... Args>
  constexpr inline void log(Args&&... args)
  {
    // Check if we need to update the index file
    // For a new format string, we need to update the index file
    constexpr char const* Name = F;
    if (!binary_log::binary_log::strings_equal(Name, "{}")
        && m_format_string_table.find(Name) == m_format_string_table.end())
    {
      // SPEC:
      // <format-string-length> <format-string> <number-of-arguments>

      m_format_string_table[Name] = ++m_format_string_index;

      // Write the length of the format string
      constexpr uint8_t format_string_length =
          binary_log::binary_log::string_length(Name);
      fwrite(&format_string_length, 1, 1, m_index_file);

      // Write the format string
      fwrite(F, 1, format_string_length, m_index_file);

      // Write the number of args taken by the format string
      constexpr uint8_t num_args = sizeof...(args);
      fwrite(&num_args, 1, 1, m_index_file);
    }

    // Write to the main log file
    // SPEC:
    // <format-string-index> <arg1> <arg2> ... <argN>
    // <format-string-index> is the index of the format string in the index file
    // <arg1> <arg2> ... <argN> are the arguments to the format string
    //
    // Each <arg> is a pair: <type, value>

    // Write the format string index
    if constexpr (!binary_log::binary_log::strings_equal(Name, "{}")) {
      uint8_t format_string_index = m_format_string_table[Name];
      fwrite(&format_string_index, 1, 1, m_log_file);
    } else {
      constexpr uint8_t format_string_index = 0;
      fwrite(&format_string_index, 1, 1, m_log_file);
    }

    // Write the args
    if constexpr (sizeof...(args) > 0) {
      pack_args(std::forward<Args>(args)...);
    }
  }
};

}  // namespace binary_log

#define BINARY_LOG(logger, format_string, ...) \
  logger.log<format_string>(__VA_ARGS__);
