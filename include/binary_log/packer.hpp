#pragma once
#include <array>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <binary_log/concepts.hpp>
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
};

}  // namespace binary_log
