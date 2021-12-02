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
struct packer
{
  template<typename T>
  static inline void write_arg_value(std::FILE* f, T&& input) = delete;

  static inline void write_arg_value(std::FILE* f, const char* input)
  {
    write_arg_value(f, static_cast<uint8_t>(std::strlen(input)));
    fwrite(input, sizeof(char), std::strlen(input), f);
  }

  template<typename T>
  requires is_numeric_type<T> static inline void write_arg_value(std::FILE* f,
                                                                 T&& input)
  {
    fwrite(&input, sizeof(T), 1, f);
  }

  template<typename T>
  requires is_string_type<T> static inline void write_arg_value(std::FILE* f,
                                                                T&& input)
  {
    write_arg_value(f, static_cast<uint8_t>(input.size()));
    fwrite(input.data(), sizeof(char), input.size(), f);
  }

  template<typename T>
  constexpr static inline void pack_arg(std::FILE* f, T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      write_arg_value(f, std::forward<T>(input));
    }
  }

  template<class... Args>
  constexpr static inline void update_log_file([[maybe_unused]] std::FILE* f,
                                               Args&&... args)
  {
    ((void)pack_arg(f, std::forward<Args>(args)), ...);
  }

  template<fmt_arg_type T>
  static inline void write_arg_type(std::FILE* f)
  {
    constexpr uint8_t type_byte = static_cast<uint8_t>(T);
    fwrite(&type_byte, sizeof(uint8_t), 1, f);
  }

  template<typename T>
  constexpr static inline void save_arg_type(std::FILE* f)
  {
    using type = typename std::decay<T>::type;

    if constexpr (is_specialization<type, constant> {}) {
      // This is a constant
      using inner_type = typename T::type;
      write_arg_type<binary_log::get_arg_type<inner_type>()>(f);
    } else {
      write_arg_type<binary_log::get_arg_type<type>()>(f);
    }
  }

  template<typename T>
  constexpr static inline void save_arg_constness(std::FILE* f, T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      constexpr bool is_constant = false;
      fwrite(&is_constant, sizeof(bool), 1, f);
    } else {
      constexpr bool is_constant = true;
      fwrite(&is_constant, sizeof(bool), 1, f);
      write_arg_value(f, input.value);
    }
  }

  template<class... Args>
  constexpr static inline void update_index_file([[maybe_unused]] std::FILE* f,
                                                 Args&&... args)
  {
    ((void)save_arg_type<Args>(f), ...);
    ((void)save_arg_constness(f, std::forward<Args>(args)), ...);
  }
};

}  // namespace binary_log
