#pragma once
#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

#include <binary_log/constant.hpp>
#include <binary_log/detail/args.hpp>
#include <binary_log/detail/concepts.hpp>

namespace binary_log
{
template<std::size_t buffer_size, typename format_string_index_type = uint8_t>
class packer
{
  std::FILE* m_log_file;
  std::FILE* m_index_file;

  // This buffer is buffering fwrite calls
  // to the log file.
  //
  // fwrite already has an internal buffer
  // but this buffer is used to avoid
  // multiple fwrite calls.
  std::array<uint8_t, buffer_size> m_buffer;
  std::size_t m_buffer_index = 0;

  template<typename T>
  void buffer_or_write(T* input, std::size_t size)
  {
    if (m_buffer_index + size >= buffer_size) {
      fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
      m_buffer_index = 0;
    }

    const auto* byte_array = reinterpret_cast<const uint8_t*>(input);
    for (std::size_t i = 0; i < size; ++i)
      m_buffer[m_buffer_index++] = byte_array[i];
  }

public:
  packer(const char* path)
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

  ~packer()
  {
    flush();
    fclose(m_log_file);
    fclose(m_index_file);
  }

  void flush_log_file()
  {
    if (m_buffer_index) {
      fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
      m_buffer_index = 0;
    }
    fflush(m_log_file);
  }

  void flush_index_file()
  {
    fflush(m_index_file);
  }

  void flush()
  {
    flush_index_file();
    flush_log_file();
  }

  template<typename T>
  inline void write_arg_value_to_log_file(T&& input) = delete;

  inline void write_arg_value_to_log_file(const char* input)
  {
    const uint8_t size = static_cast<uint8_t>(std::strlen(input));
    write_arg_value_to_log_file(size);
    buffer_or_write(input, size);
  }

  inline void write_arg_value_to_log_file(char input)
  {
    buffer_or_write(&input, sizeof(char));
  }

  inline void write_arg_value_to_log_file(bool input)
  {
    buffer_or_write(&input, sizeof(bool));
  }

  inline void write_arg_value_to_log_file(uint8_t input)
  {
    constexpr uint8_t bytes = 1;
    buffer_or_write(&bytes, sizeof(uint8_t));
    buffer_or_write(&input, sizeof(uint8_t));
  }

  inline void write_arg_value_to_log_file(uint16_t input)
  {
    if (input <= std::numeric_limits<uint8_t>::max()) {
      uint8_t value = static_cast<uint8_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 2;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(uint16_t));
    }
  }

  inline void write_arg_value_to_log_file(uint32_t input)
  {
    if (input <= std::numeric_limits<uint16_t>::max()) {
      uint16_t value = static_cast<uint16_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 4;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(uint32_t));
    }
  }

  inline void write_arg_value_to_log_file(uint64_t input)
  {
    if (input <= std::numeric_limits<uint32_t>::max()) {
      uint32_t value = static_cast<uint32_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 8;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(uint64_t));
    }
  }

  inline void write_arg_value_to_log_file(int8_t input)
  {
    constexpr uint8_t bytes = 1;
    buffer_or_write(&bytes, sizeof(uint8_t));
    buffer_or_write(&input, sizeof(int8_t));
  }

  inline void write_arg_value_to_log_file(int16_t input)
  {
    if (input <= std::numeric_limits<int8_t>::max()
        && input >= std::numeric_limits<int8_t>::min())
    {
      int8_t value = static_cast<int8_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 2;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(int16_t));
    }
  }

  inline void write_arg_value_to_log_file(int32_t input)
  {
    if (input <= std::numeric_limits<int16_t>::max()
        && input >= std::numeric_limits<int16_t>::min())
    {
      int16_t value = static_cast<int16_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 4;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(int32_t));
    }
  }

  inline void write_arg_value_to_log_file(int64_t input)
  {
    if (input <= std::numeric_limits<int32_t>::max()
        && input >= std::numeric_limits<int32_t>::min())
    {
      int32_t value = static_cast<int32_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 8;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(int64_t));
    }
  }

  inline void write_arg_value_to_log_file(float input)
  {
    buffer_or_write(&input, sizeof(float));
  }

  inline void write_arg_value_to_log_file(double input)
  {
    buffer_or_write(&input, sizeof(double));
  }

  template<typename T>
  requires is_string_type<T> inline void write_arg_value_to_log_file(T&& input)
  {
    const uint8_t size = static_cast<uint8_t>(input.size());
    write_arg_value_to_log_file(size);
    buffer_or_write(input.data(), size);
  }

  template<typename T>
  constexpr inline void pack_arg(T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      write_arg_value_to_log_file(std::forward<T>(input));
    }
  }

  constexpr inline void pack_format_string_index(
      format_string_index_type& index)
  {
    buffer_or_write(&index, sizeof(format_string_index_type));
  }

  template<class... Args>
  constexpr inline void update_log_file(Args&&... args)
  {
    ((void)pack_arg(std::forward<Args>(args)), ...);
  }

  template<fmt_arg_type T>
  inline void write_arg_type()
  {
    constexpr uint8_t type_byte = static_cast<uint8_t>(T);
    fwrite(&type_byte, sizeof(uint8_t), 1, m_index_file);
  }

  template<typename T>
  constexpr inline void save_arg_type()
  {
    using type = typename std::decay<T>::type;

    if constexpr (is_specialization<type, constant> {}) {
      // This is a constant
      using inner_type = typename T::type;
      write_arg_type<binary_log::get_arg_type<inner_type>()>();
    } else {
      write_arg_type<binary_log::get_arg_type<type>()>();
    }
  }

  template<typename T>
  inline void write_arg_value_to_index_file(T&& input) = delete;

  inline void write_arg_value_to_index_file(const char* input)
  {
    write_arg_value_to_index_file(static_cast<uint8_t>(std::strlen(input)));
    fwrite(input, sizeof(char), std::strlen(input), m_index_file);
  }

  template<typename T>
  requires is_numeric_type<T> inline void write_arg_value_to_index_file(
      T&& input)
  {
    fwrite(&input, sizeof(T), 1, m_index_file);
  }

  template<typename T>
  requires is_string_type<T> inline void write_arg_value_to_index_file(
      T&& input)
  {
    const uint8_t size = static_cast<uint8_t>(input.size());
    fwrite(&size, sizeof(uint8_t), 1, m_index_file);
    fwrite(input.data(), sizeof(char), input.size(), m_index_file);
  }

  template<typename T>
  constexpr inline void save_arg_constness(T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      constexpr bool is_constant = false;
      fwrite(&is_constant, sizeof(bool), 1, m_index_file);
    } else {
      constexpr bool is_constant = true;
      fwrite(&is_constant, sizeof(bool), 1, m_index_file);
      write_arg_value_to_index_file(input.value);
    }
  }

  template<class... Args>
  constexpr inline void update_index_file(Args&&... args)
  {
    ((void)save_arg_type<Args>(), ...);
    ((void)save_arg_constness(std::forward<Args>(args)), ...);
  }

  constexpr inline void write_format_string_to_index_file(
      const std::string_view& format_string)
  {
    const uint8_t length = format_string.size();
    fwrite(&length, sizeof(uint8_t), 1, m_index_file);
    fwrite(format_string.data(), sizeof(char), length, m_index_file);
  }

  constexpr inline void write_num_args_to_index_file(const uint8_t& num_args)
  {
    fwrite(&num_args, sizeof(uint8_t), 1, m_index_file);
  }
};

}  // namespace binary_log
