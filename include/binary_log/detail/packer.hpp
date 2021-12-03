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
template<std::size_t buffer_size>
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
    if (m_buffer_index) {
      fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
      m_buffer_index = 0;
    }

    fclose(m_log_file);
    fclose(m_index_file);
  }

  template<typename T>
  inline void write_arg_value_to_log_file(T&& input) = delete;

  inline void write_arg_value_to_log_file(const char* input)
  {
    const uint8_t size = static_cast<uint8_t>(std::strlen(input));
    write_arg_value_to_log_file(size);
    buffer_or_write(input, size);
  }

  template<typename T>
  requires is_numeric_type<T> inline void write_arg_value_to_log_file(T&& input)
  {
    buffer_or_write(&input, sizeof(T) / sizeof(uint8_t));
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

  constexpr inline void pack_format_string_index(uint8_t& index)
  {
    buffer_or_write(&index, sizeof(uint8_t));
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
