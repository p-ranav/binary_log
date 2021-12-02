#pragma once
#include <array>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <binary_log/concepts.hpp>
#include <binary_log/constant.hpp>
#include <binary_log/args.hpp>

namespace binary_log
{
struct packer
{
  template<typename T>
  static inline void pack_data(std::FILE* f, T&& input) = delete;

  static inline void pack_data(std::FILE* f, const char* input)
  {
    pack_data(f, static_cast<uint8_t>(std::strlen(input)));
    fwrite(input, sizeof(char), std::strlen(input), f);
  }

  template<typename T>
  requires is_numeric_type<T> static inline void pack_data(std::FILE* f,
                                                           T&& input)
  {
    fwrite(&input, sizeof(T), 1, f);
  }

  template<typename T>
  requires is_string_type<T> static inline void pack_data(std::FILE* f,
                                                          T&& input)
  {
    pack_data(f, static_cast<uint8_t>(input.size()));
    fwrite(input.data(), sizeof(char), input.size(), f);
  }

  template<typename T>
  constexpr static inline void pack_arg(std::FILE* f, T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      pack_data(f, std::forward<T>(input));
    }
  }

  template<class... Args>
  constexpr static inline void pack_args(std::FILE* f, Args&&... args)
  {
    ((void) pack_arg(f, std::forward<Args>(args)), ...);
  }

  template<typename T>
  constexpr static inline void pack_arg_type(std::FILE* f, T&& first)
  {
    using type = typename std::decay<T>::type;
    if constexpr (std::is_same_v<type, bool>) {
      write_arg_type<fmt_arg_type::type_bool>(f);
    } else if constexpr (std::is_same_v<type, char>) {
      write_arg_type<fmt_arg_type::type_char>(f);
    } else if constexpr (std::is_same_v<type, uint8_t>) {
      write_arg_type<fmt_arg_type::type_uint8>(f);
    } else if constexpr (std::is_same_v<type, uint16_t>) {
      write_arg_type<fmt_arg_type::type_uint16>(f);
    } else if constexpr (std::is_same_v<type, uint32_t>) {
      write_arg_type<fmt_arg_type::type_uint32>(f);
    } else if constexpr (std::is_same_v<type, uint64_t>) {
      write_arg_type<fmt_arg_type::type_uint64>(f);
    } else if constexpr (std::is_same_v<type, int8_t>) {
      write_arg_type<fmt_arg_type::type_int8>(f);
    } else if constexpr (std::is_same_v<type, int16_t>) {
      write_arg_type<fmt_arg_type::type_int16>(f);
    } else if constexpr (std::is_same_v<type, int32_t>) {
      write_arg_type<fmt_arg_type::type_int32>(f);
    } else if constexpr (std::is_same_v<type, int64_t>) {
      write_arg_type<fmt_arg_type::type_int64>(f);
    } else if constexpr (std::is_same_v<type, float>) {
      write_arg_type<fmt_arg_type::type_float>(f);
    } else if constexpr (std::is_same_v<type, double>) {
      write_arg_type<fmt_arg_type::type_double>(f);
    } else if constexpr (std::is_same_v<type, const char*>) {
      write_arg_type<fmt_arg_type::type_string>(f);
    } else if constexpr (std::is_same_v<type, std::string>) {
      write_arg_type<fmt_arg_type::type_string>(f);
    } else if constexpr (std::is_same_v<type, std::string_view>) {
      write_arg_type<fmt_arg_type::type_string>(f);
    } else if constexpr (is_specialization<type, constant> {}) {
      // This is a constant
      using inner_type = typename T::type;
      pack_arg_type<inner_type>(f, std::forward<inner_type>(first.value));
    } else {
      []<bool flag = false>()
      {
        static_assert(flag, "unsupported type");
      }
      ();
    }
  }

  template<class... Args>
  constexpr static inline void pack_arg_types(std::FILE* f, Args&&... args)
  {
    ((void) pack_arg_type(f, std::forward<Args>(args)), ...);
  }

  template<typename T>
  constexpr static inline void pack_arg_in_index_file(std::FILE* f, T&& input)
  {
    if constexpr (is_specialization<decltype(input), constant> {}) {
      constexpr bool is_constant = true;
      fwrite(&is_constant, sizeof(bool), 1, f);
      pack_data(f, input.value);
    } else {
      constexpr bool is_constant = false;
      fwrite(&is_constant, sizeof(bool), 1, f);
    }
  }

  template<class... Args>
  constexpr static inline void pack_args_in_index_file(std::FILE* f, Args&&... args)
  {
    ((void) pack_arg_in_index_file(f, std::forward<Args>(args)), ...);
  }

};

}  // namespace binary_log
