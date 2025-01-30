#pragma once
#include <cstdint>
#include <type_traits>

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
  type_uint128,
  type_int8,
  type_int16,
  type_int32,
  type_int64,
  type_int128,
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
    case fmt_arg_type::type_uint128:
      return 16;
    case fmt_arg_type::type_int8:
      return sizeof(int8_t);
    case fmt_arg_type::type_int16:
      return sizeof(int16_t);
    case fmt_arg_type::type_int32:
      return sizeof(int32_t);
    case fmt_arg_type::type_int64:
      return sizeof(int64_t);
    case fmt_arg_type::type_int128:
      return 16;
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

template<typename U>
constexpr inline fmt_arg_type get_arg_type() requires (std::is_same_v<U, std::size_t>) // && !std::is_same_v<U, uint64_t> && !std::is_same_v<U, uint32_t>)
{
  if constexpr (sizeof(std::size_t) == sizeof(uint64_t))
    return fmt_arg_type::type_uint64;
  else if constexpr (sizeof(std::size_t) == sizeof(uint32_t))
    return fmt_arg_type::type_uint32;
  else
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

template<class T, class... Ts>
constexpr static inline bool all_args_are_constants()
{
  if constexpr (is_specialization<T, constant> {}) {
    constexpr auto num_args = sizeof...(Ts);
    if constexpr (num_args == 0) {
      return true;
    } else {
      return all_args_are_constants<Ts...>();
    }
  } else {
    return false;
  }
}

}  // namespace binary_log


#pragma once
#include <array>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

// #include <binary_log/constant.hpp>
// #include <binary_log/detail/args.hpp>

namespace binary_log
{
template<size_t buffer_size= 1 * 1024 * 1024, size_t index_buffer_size = 32, size_t runlength_buffer_size = 32>
class packer
{
  std::filesystem::path m_path;
  std::FILE* m_log_file;
  std::FILE* m_index_file;
  std::FILE* m_runlength_file;

  // This buffer is buffering fwrite calls
  // to the log file.
  //
  // fwrite already has an internal buffer
  // but this buffer is used to avoid
  // multiple fwrite calls.
  std::array<uint8_t, buffer_size> m_buffer;
  std::size_t m_buffer_index = 0;

  std::array<uint8_t, index_buffer_size> m_index_buffer;
  std::size_t m_index_buffer_index = 0;

  // Members for run-length encoding
  // of the log file.
  bool m_first_call = true;
  std::array<uint8_t, runlength_buffer_size> m_runlength_buffer;
  std::size_t m_runlength_buffer_index = 0;
  std::size_t m_runlength_index = 0;
  uint64_t m_current_runlength = 0;

  template<typename T, std::size_t size>
  void buffer_or_write(T* input)
  {
    std::size_t bytes_left = size;
    auto* byte_array = reinterpret_cast<const uint8_t*>(input);
    while (bytes_left) {
      if (m_buffer_index + bytes_left >= buffer_size) {
        fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
        m_buffer_index = 0;
      }

      std::size_t bytes_to_copy = std::min(bytes_left, buffer_size);
      std::copy_n(byte_array, bytes_to_copy, &m_buffer[m_buffer_index]);
      byte_array += bytes_to_copy;
      m_buffer_index += bytes_to_copy;
      bytes_left -= bytes_to_copy;
    }
  }

  template<typename T>
  void buffer_or_write(T* input, std::size_t size)
  {
    std::size_t bytes_left = size;
    auto* byte_array = reinterpret_cast<const uint8_t*>(input);
    while (bytes_left) {
      if (m_buffer_index + bytes_left >= buffer_size) {
        fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
        m_buffer_index = 0;
      }

      std::size_t bytes_to_copy = std::min(bytes_left, buffer_size);
      std::copy_n(byte_array, bytes_to_copy, &m_buffer[m_buffer_index]);
      byte_array += bytes_to_copy;
      m_buffer_index += bytes_to_copy;
      bytes_left -= bytes_to_copy;
    }
  }

  template<typename T>
  constexpr void buffer_or_write_index_file(T* input, std::size_t size)
  {
    std::size_t bytes_left = size;
    const auto* byte_array = reinterpret_cast<const uint8_t*>(input);
    while (bytes_left) {
      if (m_index_buffer_index + bytes_left >= index_buffer_size) {
        fwrite(m_index_buffer.data(),
               sizeof(uint8_t),
               m_index_buffer_index,
               m_index_file);
        m_index_buffer_index = 0;
      }

      std::size_t bytes_to_copy = std::min(bytes_left, index_buffer_size-1);
      std::copy_n(byte_array, bytes_to_copy, &m_index_buffer[m_index_buffer_index]);
      byte_array += bytes_to_copy;
      m_index_buffer_index += bytes_to_copy;
      bytes_left -= bytes_to_copy;
    }
  }

public:
  packer(const std::filesystem::path& path) : m_path(path)
  {
    // Create the log file
    // All the log contents go here
    m_log_file = fopen(get_path().c_str(), "wb");
    if (m_log_file == nullptr) {
#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711L
      throw std::invalid_argument("fopen failed");
#else
      abort();
#endif
    }

    // No fwrite buffering
    setvbuf(m_log_file, nullptr, _IONBF, 0);

    // Create the index file
    m_index_file = fopen(get_index_path().c_str(), "wb");
    if (m_index_file == nullptr) {
#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711L
      throw std::invalid_argument("fopen failed");
#else
      abort();
#endif
    }

    // No fwrite buffering
    setvbuf(m_index_file, nullptr, _IONBF, 0);

    // Create the runlength file
    m_runlength_file = fopen(get_runlength_path().c_str(), "wb");
    if (m_runlength_file == nullptr) {
#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711L
      throw std::invalid_argument("fopen failed");
#else
      abort();
#endif
    }

    m_runlength_index = 0;
    m_current_runlength = 0;
  }

  packer(const char* path) : packer(std::filesystem::path {path})
  {
  }

  ~packer()
  {
    flush();
    fclose(m_log_file);
    fclose(m_index_file);
    fclose(m_runlength_file);
  }

  std::filesystem::path get_path() const
  {
    return m_path;
  }

  std::filesystem::path get_index_path() const
  {
    std::filesystem::path index_file_path = m_path;
    index_file_path.replace_extension(m_path.extension().string() + ".index");
    return index_file_path;
  }

  std::filesystem::path get_runlength_path() const
  {
    std::filesystem::path runlength_file_path = m_path;
    runlength_file_path.replace_extension(m_path.extension().string() + ".runlength");
    return runlength_file_path;
  }

  void flush_log_file()
  {
    if (m_log_file == nullptr) {
      return;
    }
    fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
    m_buffer_index = 0;
    fflush(m_log_file);
  }

  void flush_index_file()
  {
    if (m_index_file == nullptr) {
      return;
    }
    fwrite(m_index_buffer.data(),
           sizeof(uint8_t),
           m_index_buffer_index,
           m_index_file);
    m_index_buffer_index = 0;
    fflush(m_index_file);
  }

  void flush_runlength_file()
  {
    if (m_runlength_file == nullptr) {
      return;
    }
    fwrite(m_runlength_buffer.data(),
           sizeof(uint8_t),
           m_runlength_buffer_index,
           m_runlength_file);
    m_runlength_buffer_index = 0;
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
    uint16_t size = static_cast<uint16_t>(std::strlen(input));
    buffer_or_write<uint16_t, sizeof(uint16_t)>(&size);
    buffer_or_write(input, size);
  }

  inline void write_arg_value_to_log_file(char input)
  {
    buffer_or_write<char, sizeof(char)>(&input);
  }

  inline void write_arg_value_to_log_file(bool input)
  {
    buffer_or_write<bool, sizeof(bool)>(&input);
  }

  inline void write_arg_value_to_log_file(uint8_t input)
  {
    buffer_or_write<uint8_t, sizeof(uint8_t)>(&input);
  }

  inline void write_arg_value_to_log_file(uint16_t input)
  {
    buffer_or_write<uint16_t, sizeof(uint16_t)>(&input);
  }

  inline void write_arg_value_to_log_file(uint32_t input)
  {
    buffer_or_write<uint32_t, sizeof(uint32_t)>(&input);
  }

  inline void write_arg_value_to_log_file(uint64_t input)
  {
    buffer_or_write<uint64_t, sizeof(uint64_t)>(&input);
  }

  template<typename U>
  inline void write_arg_value_to_log_file(U &input) requires (std::is_same_v<U, std::size_t>) // && !std::is_same_v<U, uint64_t> && !std::is_same_v<U, uint32_t>)
  {
    buffer_or_write<U, sizeof(U)>(&input);
  }

  inline void write_arg_value_to_log_file(int8_t input)
  {
    buffer_or_write<int8_t, sizeof(int8_t)>(&input);
  }

  inline void write_arg_value_to_log_file(int16_t input)
  {
    buffer_or_write<int16_t, sizeof(int16_t)>(&input);
  }

  inline void write_arg_value_to_log_file(int32_t input)
  {
    buffer_or_write<int32_t, sizeof(int32_t)>(&input);
  }

  inline void write_arg_value_to_log_file(int64_t input)
  {
    buffer_or_write<int64_t, sizeof(int64_t)>(&input);
  }

  inline void write_arg_value_to_log_file(float input)
  {
    buffer_or_write<float, sizeof(float)>(&input);
  }

  inline void write_arg_value_to_log_file(double input)
  {
    buffer_or_write<double, sizeof(double)>(&input);
  }

  template<typename T>
  requires is_string_type<T> inline void write_arg_value_to_log_file(T&& input)
  {
    uint16_t size = static_cast<uint16_t>(input.size());
    buffer_or_write<uint16_t, sizeof(uint16_t)>(&size);
    buffer_or_write(input.data(), size);
  }

  template<typename T>
  constexpr inline void pack_arg(T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      write_arg_value_to_log_file(std::forward<T>(input));
    }
  }

  inline void write_current_runlength_to_runlength_file()
  {
    if (m_current_runlength > 1) {
      size_t bytes_left = sizeof(uint16_t) + sizeof(uint64_t);
      // make the bytes we'll write to the runlength file
      uint8_t bytes[sizeof(uint16_t) + sizeof(uint64_t)];
      // fill the bytes
      std::memcpy(&bytes[0], &m_runlength_index, sizeof(uint16_t));
      std::memcpy(&bytes[sizeof(uint16_t)], &m_current_runlength, sizeof(uint64_t));
      // write the bytes
      while (bytes_left) {
        if (m_runlength_buffer_index + bytes_left >= runlength_buffer_size) {
          fwrite(m_runlength_buffer.data(),
                 sizeof(uint8_t),
                 m_runlength_buffer_index,
                 m_runlength_file);
          m_runlength_buffer_index = 0;
        }

        std::size_t bytes_to_copy = std::min(bytes_left, runlength_buffer_size);
        std::copy_n(bytes, bytes_to_copy, &m_runlength_buffer[m_runlength_buffer_index]);
        m_runlength_buffer_index += bytes_to_copy;
        bytes_left -= bytes_to_copy;
      }
    }
  }

  constexpr inline void pack_format_string_index(uint16_t index)
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
        buffer_or_write<uint16_t, sizeof(uint16_t)>(&index);
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
        buffer_or_write<uint16_t, sizeof(uint16_t)>(&index);
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
  constexpr inline void write_arg_type()
  {
    constexpr uint8_t type_byte = static_cast<uint8_t>(T);
    buffer_or_write_index_file(&type_byte, sizeof(uint8_t));
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
    const uint16_t size = static_cast<uint16_t>(std::strlen(input));
    buffer_or_write_index_file(&size, sizeof(uint16_t));
    buffer_or_write_index_file(input, size);
  }

  template<typename T>
  requires is_numeric_type<T> inline void write_arg_value_to_index_file(
      T&& input)
  {
    buffer_or_write_index_file(&input, sizeof(T));
  }

  template<typename T>
  requires is_string_type<T> inline void write_arg_value_to_index_file(
      T&& input)
  {
    const uint16_t size = static_cast<uint16_t>(input.size());
    buffer_or_write_index_file(&size, sizeof(uint16_t));
    buffer_or_write_index_file(input.data(), input.size());
  }

  template<typename T>
  constexpr inline void save_arg_constness(T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      constexpr bool is_constant = false;
      buffer_or_write_index_file(&is_constant, sizeof(bool));
    } else {
      constexpr bool is_constant = true;
      buffer_or_write_index_file(&is_constant, sizeof(bool));
      write_arg_value_to_index_file(input.value);
    }
  }

  template<class... Args>
  constexpr inline void update_index_file(Args&&... args)
  {
    ((void)save_arg_type<Args>(), ...);
    ((void)save_arg_constness(std::forward<Args>(args)), ...);
  }

  template<const char* format_string>
  void write_format_string_to_index_file()
  {
    const uint16_t length = static_cast<uint16_t>(std::strlen(format_string));
    buffer_or_write_index_file(&length, sizeof(uint16_t));
    buffer_or_write_index_file(format_string, length);
  }

  constexpr inline void write_num_args_to_index_file(const uint8_t& num_args)
  {
    buffer_or_write_index_file(&num_args, sizeof(uint8_t));
  }
};

}  // namespace binary_log


#pragma once
#include <filesystem>
#include <iostream>
#include <string>

// #include <binary_log/constant.hpp>
// #include <binary_log/detail/args.hpp>
// #include <binary_log/detail/packer.hpp>

namespace binary_log
{
template<size_t buffer_size = 1 * 1024 * 1024, size_t index_buffer_size = 32>
class binary_log
{
  packer<buffer_size, index_buffer_size> m_packer;
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

  const packer<buffer_size, index_buffer_size>& get_packer() const
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
    m_packer.template write_format_string_to_index_file<format_string>();

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


