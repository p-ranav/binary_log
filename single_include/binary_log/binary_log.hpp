#pragma once
#include <cstdint>

// #include <binary_log/constant.hpp>

namespace binary_log
{
template<typename T>
struct constant
{
  using type = T;
  const T value;
  constexpr constant(T v)
      : value(v)
  {
  }
};

}  // namespace binary_log
// #include <binary_log/detail/concepts.hpp>
#include <concepts>
#include <string>
#include <string_view>

namespace binary_log
{
template<typename T, typename... U>
concept is_any_of = (std::same_as<T, U> || ...);

template<typename T>
concept is_numeric_type =
    is_any_of<std::remove_cvref_t<std::remove_pointer_t<std::decay_t<T>>>,
              bool,
              char,
              uint8_t,
              int8_t,
              uint16_t,
              int16_t,
              uint32_t,
              int32_t,
              uint64_t,
              int64_t,
              float,
              double>;

template<typename T>
concept is_string_type =
    is_any_of<std::remove_cvref_t<std::remove_pointer_t<std::decay_t<T>>>,
              std::string,
              std::string_view>;

}  // namespace binary_log
// #include <binary_log/detail/is_specialization.hpp>
#include <type_traits>

namespace binary_log
{
template<class T, template<class...> class Template>
struct is_specialization : std::false_type
{
};

template<template<class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type
{
};

}  // namespace binary_log

namespace binary_log
{
enum class fmt_arg_type
{
  type_bool,
  type_char,
  type_uint8,
  type_uint16,
  type_uint32,
  type_uint64,
  type_int8,
  type_int16,
  type_int32,
  type_int64,
  type_float,
  type_double,
  type_string,
};

static inline std::size_t sizeof_arg_type(fmt_arg_type type)
{
  switch (type) {
    case fmt_arg_type::type_bool:
      return sizeof(bool);
    case fmt_arg_type::type_char:
      return sizeof(char);
    case fmt_arg_type::type_uint8:
      return sizeof(uint8_t);
    case fmt_arg_type::type_uint16:
      return sizeof(uint16_t);
    case fmt_arg_type::type_uint32:
      return sizeof(uint32_t);
    case fmt_arg_type::type_uint64:
      return sizeof(uint64_t);
    case fmt_arg_type::type_int8:
      return sizeof(int8_t);
    case fmt_arg_type::type_int16:
      return sizeof(int16_t);
    case fmt_arg_type::type_int32:
      return sizeof(int32_t);
    case fmt_arg_type::type_int64:
      return sizeof(int64_t);
    case fmt_arg_type::type_float:
      return sizeof(float);
    case fmt_arg_type::type_double:
      return sizeof(double);
    case fmt_arg_type::type_string:
      return sizeof(char);
    default:
      return 0;
  }
}

template<typename T>
constexpr static inline fmt_arg_type get_arg_type() = delete;

template<>
constexpr inline fmt_arg_type get_arg_type<bool>()
{
  return fmt_arg_type::type_bool;
}

template<>
constexpr inline fmt_arg_type get_arg_type<char>()
{
  return fmt_arg_type::type_char;
}

template<>
constexpr inline fmt_arg_type get_arg_type<uint8_t>()
{
  return fmt_arg_type::type_uint8;
}

template<>
constexpr inline fmt_arg_type get_arg_type<uint16_t>()
{
  return fmt_arg_type::type_uint16;
}

template<>
constexpr inline fmt_arg_type get_arg_type<uint32_t>()
{
  return fmt_arg_type::type_uint32;
}

template<>
constexpr inline fmt_arg_type get_arg_type<uint64_t>()
{
  return fmt_arg_type::type_uint64;
}

template<>
constexpr inline fmt_arg_type get_arg_type<int8_t>()
{
  return fmt_arg_type::type_int8;
}

template<>
constexpr inline fmt_arg_type get_arg_type<int16_t>()
{
  return fmt_arg_type::type_int16;
}

template<>
constexpr inline fmt_arg_type get_arg_type<int32_t>()
{
  return fmt_arg_type::type_int32;
}

template<>
constexpr inline fmt_arg_type get_arg_type<int64_t>()
{
  return fmt_arg_type::type_int64;
}

template<>
constexpr inline fmt_arg_type get_arg_type<float>()
{
  return fmt_arg_type::type_float;
}

template<>
constexpr inline fmt_arg_type get_arg_type<double>()
{
  return fmt_arg_type::type_double;
}

template<>
constexpr inline fmt_arg_type get_arg_type<const char*>()
{
  return fmt_arg_type::type_string;
}

template<typename T>
requires is_string_type<T> constexpr inline fmt_arg_type get_arg_type()
{
  return fmt_arg_type::type_string;
}

constexpr static inline bool all_args_are_constants()
{
  return true;
}

template<class T, class... Ts>
constexpr static inline bool all_args_are_constants(T&&, Ts&&... rest)
{
  if constexpr (is_specialization<T, constant> {}) {
    return all_args_are_constants(std::forward<Ts>(rest)...);
  } else {
    return false;
  }
}

}  // namespace binary_log

#pragma once
#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

// #include <binary_log/constant.hpp>
// #include <binary_log/detail/args.hpp>

namespace binary_log
{
class packer
{
  std::FILE* m_log_file;
  std::FILE* m_index_file;
  std::FILE* m_runlength_file;

  // This buffer is buffering fwrite calls
  // to the log file.
  //
  // fwrite already has an internal buffer
  // but this buffer is used to avoid
  // multiple fwrite calls.
  constexpr static inline std::size_t buffer_size = 1024 * 1024;
  std::array<uint8_t, buffer_size> m_buffer;
  std::size_t m_buffer_index = 0;

  // Members for run-length encoding
  // of the log file.
  bool m_first_call = true;
  std::size_t m_runlength_index = 0;
  uint64_t m_current_runlength = 0;

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

    // Create the runlength file
    std::string runlength_file_path = std::string {path} + ".runlength";
    m_runlength_file = fopen(runlength_file_path.data(), "wb");
    if (m_runlength_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }
  }

  ~packer()
  {
    flush();
    fclose(m_log_file);
    fclose(m_index_file);
    fclose(m_runlength_file);
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

  void flush_runlength_file()
  {
    write_current_runlength_to_runlength_file();
    fflush(m_runlength_file);
  }

  void flush()
  {
    flush_index_file();
    flush_log_file();
    flush_runlength_file();
  }

  template<typename T>
  inline void write_arg_value_to_log_file(T&& input) = delete;

  inline void write_arg_value_to_log_file(const char* input)
  {
    const uint8_t size = static_cast<uint8_t>(std::strlen(input));
    buffer_or_write(&size, sizeof(uint8_t));
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
    buffer_or_write(&size, sizeof(uint8_t));
    buffer_or_write(input.data(), size);
  }

  template<typename T>
  constexpr inline void pack_arg(T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      write_arg_value_to_log_file(std::forward<T>(input));
    }
  }

  constexpr inline void write_current_runlength_to_runlength_file()
  {
    if (m_current_runlength > 1) {
      const uint8_t index = static_cast<uint8_t>(m_runlength_index);
      fwrite(&index, sizeof(uint8_t), 1, m_runlength_file);

      // Write runlength to file
      // Perform integer compression
      if (m_current_runlength <= std::numeric_limits<uint8_t>::max()) {
        uint8_t value = static_cast<uint8_t>(m_current_runlength);
        constexpr uint8_t bytes = 1;
        fwrite(&bytes, sizeof(uint8_t), 1, m_runlength_file);
        fwrite(&value, sizeof(uint8_t), 1, m_runlength_file);
      } else if (m_current_runlength <= std::numeric_limits<uint16_t>::max()) {
        uint16_t value = static_cast<uint16_t>(m_current_runlength);
        constexpr uint8_t bytes = 2;
        fwrite(&bytes, sizeof(uint8_t), 1, m_runlength_file);
        fwrite(&value, sizeof(uint16_t), 1, m_runlength_file);
      } else if (m_current_runlength <= std::numeric_limits<uint32_t>::max()) {
        uint32_t value = static_cast<uint32_t>(m_current_runlength);
        constexpr uint8_t bytes = 4;
        fwrite(&bytes, sizeof(uint8_t), 1, m_runlength_file);
        fwrite(&value, sizeof(uint32_t), 1, m_runlength_file);
      } else {
        uint64_t value = static_cast<uint64_t>(m_current_runlength);
        constexpr uint8_t bytes = 8;
        fwrite(&bytes, sizeof(uint8_t), 1, m_runlength_file);
        fwrite(&value, sizeof(uint64_t), 1, m_runlength_file);
      }
      m_current_runlength = 0;
    }
  }

  constexpr inline void pack_format_string_index(uint8_t& index)
  {
    // Evaluate this index
    //
    // If index is the same as the m_runlength_index
    // but m_current_runlength is 0, write it and increment m_current_runlength
    // else, no need to write it, just update the runlength
    //
    // If the index is different from m_runlength_index
    // then, write (m_runlength_index + m_current_runlength)
    // to the m_runlength_file and write the new index
    // to the logfile

    if (m_runlength_index == 0 && index == 0) {
      // First index
      if (m_current_runlength == 0) {
        // First call
        // Write index to log file
        buffer_or_write(&index, sizeof(uint8_t));
        m_current_runlength++;
      } else if (m_current_runlength >= 1) {
        m_current_runlength++;
      }
    } else {
      // Not first index
      if (m_runlength_index == index) {
        // No change to index
        m_current_runlength++;
      } else {
        // Write current runlength to file
        write_current_runlength_to_runlength_file();

        // Write index to log file
        buffer_or_write(&index, sizeof(uint8_t));
        m_current_runlength = 1;
        m_runlength_index = index;
      }
    }
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
    const uint8_t size = static_cast<uint8_t>(std::strlen(input));
    fwrite(&size, sizeof(uint8_t), 1, m_index_file);
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

    // Initialize the runlength member variables
    // This might be the first log call on this logger
    if (m_first_call) {
      m_runlength_index = 0;
      m_current_runlength = 0;
      m_first_call = false;
    }
  }

  constexpr inline void write_num_args_to_index_file(const uint8_t& num_args)
  {
    fwrite(&num_args, sizeof(uint8_t), 1, m_index_file);
  }
};

}  // namespace binary_log


#pragma once
#include <iostream>
#include <string>

// #include <binary_log/constant.hpp>
// #include <binary_log/detail/args.hpp>
// #include <binary_log/detail/packer.hpp>

namespace binary_log
{
class binary_log
{
  packer m_packer;
  uint8_t m_format_string_index {0};

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
    uint8_t current_index = static_cast<uint8_t>(pos);
    m_packer.pack_format_string_index(current_index);

    // Write the args
    if (num_args > 0 && !all_args_are_constants(std::forward<Args>(args)...)) {
      m_packer.update_log_file(std::forward<Args>(args)...);
    }
  }
};

}  // namespace binary_log

#define BINARY_LOG_CONCAT0(a, b) a##b
#define BINARY_LOG_CONCAT(a, b) BINARY_LOG_CONCAT0(a, b)

#define BINARY_LOG(logger, format_string, ...) \
  { \
    static std::size_t BINARY_LOG_CONCAT(__binary_log_format_string_id_pos, \
                                         __LINE__) = \
        logger.log_index(format_string, ##__VA_ARGS__); \
    logger.log(BINARY_LOG_CONCAT(__binary_log_format_string_id_pos, __LINE__), \
               ##__VA_ARGS__); \
  }


