#pragma once
#include <string>

#include <binary_log/constant.hpp>
#include <binary_log/fixed_string.hpp>
#include <binary_log/packer.hpp>
#include <binary_log/string_utils.hpp>

namespace binary_log
{
template<typename format_string_index_type = uint8_t>
class binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;
  format_string_index_type m_format_string_index {0};

  // Used for run-length encoding of log calls
  bool m_first_time {true};
  int m_last_index {-1};
  int m_num_calls_for_current_index {-1};

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
    } else if constexpr (std::is_same_v<T, const char*>) {
      packer::write_type<packer::datatype::type_cstring>(m_index_file);
    } else if constexpr (std::is_same_v<T, std::string>) {
      packer::write_type<packer::datatype::type_string>(m_index_file);
    } else if constexpr (std::is_same_v<T, std::string_view>) {
      packer::write_type<packer::datatype::type_string_view>(m_index_file);
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
    // no more log calls
    // If a run-length was not written, write it now
    if (m_num_calls_for_current_index != -1) {
      // write the number of calls for the previous format string
      fwrite(&m_num_calls_for_current_index, sizeof(uint32_t), 1, m_log_file);
      m_num_calls_for_current_index = -1;
    }

    fclose(m_log_file);
    fclose(m_index_file);
  }

  template<fixed_string F, class... Args>
  constexpr inline uint8_t log_index(Args&&... args)
  {
    // SPEC:
    // <format-string-length> <format-string>
    // <number-of-arguments> <arg-type-1> <arg-type-2> ... <arg-type-N>
    // <arg-1-is-lvalue-ref> <arg-1-value>? <arg-2-is-lvalue-ref> <arg-2-value>?
    // ...
    //
    // If the arg is not an lvalue, it is stored in the index file
    // and the value is not stored in the log file
    constexpr char const* Name = F;
    constexpr uint8_t num_args = sizeof...(Args);

    m_format_string_index++;

    // Write the length of the format string
    constexpr uint8_t format_string_length = string_length(Name);
    fwrite(&format_string_length, 1, 1, m_index_file);

    // Write the format string
    fwrite(F, 1, format_string_length, m_index_file);

    // Write the number of args taken by the format string
    fwrite(&num_args, sizeof(uint8_t), 1, m_index_file);

    // Write the type of each argument
    if constexpr (num_args > 0) {
      pack_arg_types<Args...>();
      pack_args_in_index_file(std::forward<Args>(args)...);
    }

    return m_format_string_index - 1;
  }

  template<fixed_string F, class... Args>
  constexpr inline void log(uint8_t pos, Args&&... args)
  {
    constexpr uint8_t num_args = sizeof...(Args);

    // Write to the main log file
    // SPEC:
    // <format-string-index> <arg1> <arg2> ... <argN>
    // <format-string-index> is the index of the format string in the index file
    // <arg1> <arg2> ... <argN> are the arguments to the format string
    //
    // Each <arg> is a pair: <type, value>
    //
    // Optimization 1:
    //   If all the arguments to the log call are constants, they will go in the
    //   index file. In this case, we can use run-length encoding to
    //   store the number of times the log call was made
    //   e.g., BINARY_LOG("Foo bar")
    //   If the above call is made 1 million times, we can store
    //   <format-string-index> <1000000>
    //   instead of storing <format-string-index> 1 million times

    // Write the format string index
    if (m_last_index == -1) {
      // first time
      fwrite(&pos, sizeof(uint8_t), 1, m_log_file);
      m_last_index = pos;
    } else {
      if (m_last_index != pos) {
        // new format string
        if (m_num_calls_for_current_index != -1) {
          // write the number of calls for the previous format string
          fwrite(
              &m_num_calls_for_current_index, sizeof(uint32_t), 1, m_log_file);
          m_num_calls_for_current_index = -1;
        }

        // write the new format string index
        fwrite(&pos, sizeof(uint8_t), 1, m_log_file);
        m_last_index = pos;
      }
    }

    // Write the args
    if constexpr (num_args > 0
                  && !all_args_are_constants(std::forward<Args>(args)...))
    {
      pack_args(std::forward<Args>(args)...);
      m_last_index = -1;
    } else {
      // Increment num calls for current index
      if (m_num_calls_for_current_index == -1) {
        m_num_calls_for_current_index = 1;
      } else {
        m_num_calls_for_current_index++;
      }
    }
  }
};

}  // namespace binary_log

#define CONCAT0(a, b) a##b
#define CONCAT(a, b) CONCAT0(a, b)

#define BINARY_LOG(logger, format_string, ...) \
  { \
    static uint8_t CONCAT(__binary_log_format_string_id_pos, __LINE__) = \
        logger.log_index<format_string>(__VA_ARGS__); \
    logger.log<format_string>( \
        CONCAT(__binary_log_format_string_id_pos, __LINE__), ##__VA_ARGS__); \
  }
