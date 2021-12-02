#pragma once
#include <iostream>
#include <string>

#include <binary_log/args.hpp>
#include <binary_log/constant.hpp>
#include <binary_log/packer.hpp>

namespace binary_log
{
template<typename format_string_index_type = uint8_t>
class binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;
  format_string_index_type m_format_string_index {0};

  constexpr void pack_args_in_index_file() {}

  template<typename T>
  constexpr void pack_arg_in_index_file(T&& input)
  {
    if constexpr (is_specialization<decltype(input), constant> {}) {
      constexpr bool is_constant = true;
      fwrite(&is_constant, sizeof(bool), 1, m_index_file);
      packer::pack_data(m_index_file, input.value);
    } else {
      constexpr bool is_constant = false;
      fwrite(&is_constant, sizeof(bool), 1, m_index_file);
    }
  }

  template<class T, class... Ts>
  constexpr void pack_args_in_index_file(T&& first, Ts&&... rest)
  {
    pack_arg_in_index_file(std::forward<T>(first));

    if constexpr (sizeof...(rest) > 0) {
      pack_args_in_index_file(std::forward<Ts>(rest)...);
    }
  }

  constexpr void pack_arg() {}

  template<typename T>
  constexpr void pack_arg(T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      packer::pack_data(m_log_file, std::forward<T>(input));
    }
  }

  template<class T, class... Ts>
  constexpr void pack_args(T&& first, Ts&&... rest)
  {
    pack_arg(std::forward<T>(first));

    if constexpr (sizeof...(rest) > 0) {
      pack_args(std::forward<Ts>(rest)...);
    }
  }

  constexpr void pack_args() {}

  template<typename T>
  constexpr void pack_arg_type(T&& first)
  {
    using type = typename std::decay<T>::type;
    if constexpr (std::is_same_v<type, bool>) {
      write_arg_type<fmt_arg_type::type_bool>(m_index_file);
    } else if constexpr (std::is_same_v<type, char>) {
      write_arg_type<fmt_arg_type::type_char>(m_index_file);
    } else if constexpr (std::is_same_v<type, uint8_t>) {
      write_arg_type<fmt_arg_type::type_uint8>(m_index_file);
    } else if constexpr (std::is_same_v<type, uint16_t>) {
      write_arg_type<fmt_arg_type::type_uint16>(m_index_file);
    } else if constexpr (std::is_same_v<type, uint32_t>) {
      write_arg_type<fmt_arg_type::type_uint32>(m_index_file);
    } else if constexpr (std::is_same_v<type, uint64_t>) {
      write_arg_type<fmt_arg_type::type_uint64>(m_index_file);
    } else if constexpr (std::is_same_v<type, int8_t>) {
      write_arg_type<fmt_arg_type::type_int8>(m_index_file);
    } else if constexpr (std::is_same_v<type, int16_t>) {
      write_arg_type<fmt_arg_type::type_int16>(m_index_file);
    } else if constexpr (std::is_same_v<type, int32_t>) {
      write_arg_type<fmt_arg_type::type_int32>(m_index_file);
    } else if constexpr (std::is_same_v<type, int64_t>) {
      write_arg_type<fmt_arg_type::type_int64>(m_index_file);
    } else if constexpr (std::is_same_v<type, float>) {
      write_arg_type<fmt_arg_type::type_float>(m_index_file);
    } else if constexpr (std::is_same_v<type, double>) {
      write_arg_type<fmt_arg_type::type_double>(m_index_file);
    } else if constexpr (std::is_same_v<type, const char*>) {
      write_arg_type<fmt_arg_type::type_string>(m_index_file);
    } else if constexpr (std::is_same_v<type, std::string>) {
      write_arg_type<fmt_arg_type::type_string>(m_index_file);
    } else if constexpr (std::is_same_v<type, std::string_view>) {
      write_arg_type<fmt_arg_type::type_string>(m_index_file);
    } else if constexpr (is_specialization<type, constant> {}) {
      // This is a constant
      using inner_type = typename T::type;
      pack_arg_type<inner_type>(std::forward<inner_type>(first.value));
    } else {
      []<bool flag = false>()
      {
        static_assert(flag, "unsupported type");
      }
      ();
    }
  }

  constexpr void pack_arg_types() {}

  template<class T, class... Ts>
  constexpr void pack_arg_types(T&& first, Ts&&... rest)
  {
    pack_arg_type(std::forward<T>(first));

    if constexpr (sizeof...(Ts) > 0) {
      pack_arg_types(std::forward<Ts>(rest)...);
    }
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
    // <arg-1-is-lvalue-ref> <arg-1-value>? <arg-2-is-lvalue-ref> <arg-2-value>?
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
      pack_arg_types(std::forward<Args>(args)...);
      pack_args_in_index_file(std::forward<Args>(args)...);
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
      pack_args(std::forward<Args>(args)...);
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
