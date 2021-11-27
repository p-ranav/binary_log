#pragma once
#include <iostream>
#include <set>
#include <string_view>

#include <binary_log/crc16.hpp>
#include <binary_log/fixed_string.hpp>
#include <binary_log/packer.hpp>
#include <binary_log/string_utils.hpp>

namespace binary_log
{
class binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;

  // Format string table
  std::set<uint16_t> m_format_string_table;

  template<typename T>
  constexpr void pack_arg(const T& input)
  {
    packer::pack_data(m_log_file, input);
  }

  template<class T, class... Ts>
  constexpr void pack_args(T&& first, Ts&&... rest)
  {
    pack_arg(std::forward<T>(first));

    if constexpr (sizeof...(rest) > 0) {
      pack_args(std::forward<Ts>(rest)...);
    }
  }

  template<typename T>
  constexpr void pack_arg_type()
  {
    if constexpr (std::is_same_v<T, char>) {
      packer::write_type<packer::datatype::type_char>(m_index_file);
    } else if constexpr (std::is_same_v<T, uint8_t>) {
      packer::write_type<packer::datatype::type_uint8>(m_index_file);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      packer::write_type<packer::datatype::type_uint16>(m_index_file);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      packer::write_type<packer::datatype::type_uint32>(m_index_file);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
      packer::write_type<packer::datatype::type_uint64>(m_index_file);
    } else if constexpr (std::is_same_v<T, int8_t>) {
      packer::write_type<packer::datatype::type_int8>(m_index_file);
    } else if constexpr (std::is_same_v<T, int16_t>) {
      packer::write_type<packer::datatype::type_int16>(m_index_file);
    } else if constexpr (std::is_same_v<T, int32_t>) {
      packer::write_type<packer::datatype::type_int32>(m_index_file);
    } else if constexpr (std::is_same_v<T, int64_t>) {
      packer::write_type<packer::datatype::type_int64>(m_index_file);
    } else if constexpr (std::is_same_v<T, float>) {
      packer::write_type<packer::datatype::type_float>(m_index_file);
    } else if constexpr (std::is_same_v<T, double>) {
      packer::write_type<packer::datatype::type_double>(m_index_file);
    }
  }

  template<class T, class... Ts>
  constexpr void pack_arg_types()
  {
    pack_arg_type<T>();

    if constexpr (sizeof...(Ts) > 0) {
      pack_arg_types<Ts...>();
    }
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

  template<fixed_string F, uint16_t H, class... Args>
  constexpr inline void log(Args&&... args)
  {
    // Check if we need to update the index file
    // For a new format string, we need to update the index file
    constexpr char const* Name = F;

    auto it = m_format_string_table.find(H);
    uint8_t pos = 0; /* Assumption: There will be no more than 255 unique calls
                        on this logger */

    if (it == m_format_string_table.end()) {
      // SPEC:
      // <format-string-index [0-255]> <format-string-length> <format-string>
      // <number-of-arguments> <arg-type-1> <arg-type-2> ... <arg-type-N>

      auto result = m_format_string_table.insert(H);

      it = result.first;
      pos = std::distance(m_format_string_table.begin(), it);

      fwrite(&pos, sizeof(uint8_t), 1, m_index_file);

      // Write the length of the format string
      constexpr uint8_t format_string_length = string_length(Name);
      fwrite(&format_string_length, 1, 1, m_index_file);

      // Write the format string
      fwrite(F, 1, format_string_length, m_index_file);

      // Write the number of args taken by the format string
      constexpr uint8_t num_args = sizeof...(args);
      fwrite(&num_args, 1, 1, m_index_file);

      // Write the type of each argument
      if constexpr (sizeof...(args) > 0) {
        pack_arg_types<Args...>();
      }

    } else {
      pos = std::distance(m_format_string_table.begin(), it);
    }

    // Write to the main log file
    // SPEC:
    // <format-string-index> <arg1> <arg2> ... <argN>
    // <format-string-index> is the index of the format string in the index file
    // <arg1> <arg2> ... <argN> are the arguments to the format string
    //
    // Each <arg> is a pair: <type, value>

    // Write the format string index
    fwrite(&pos, sizeof(uint8_t), 1, m_log_file);

    // Write the args
    if constexpr (sizeof...(args) > 0) {
      pack_args(std::forward<Args>(args)...);
    }
  }
};

}  // namespace binary_log

#define BINARY_LOG(logger, format_string, ...) \
  constexpr uint16_t CONCAT(format_string_id, __LINE__) = crc16( \
      format_string __AT__, binary_log::string_length(format_string __AT__)); \
  logger.log<format_string, CONCAT(format_string_id, __LINE__)>(__VA_ARGS__);
