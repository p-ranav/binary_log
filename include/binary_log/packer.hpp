#pragma once
#include <array>
#include <concepts>
#include <cstring>
#include <limits>
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

struct packer
{
  enum class datatype
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

  static inline std::size_t sizeof_datatype(datatype type)
  {
    switch (type) {
      case datatype::type_bool:
        return sizeof(bool);
      case datatype::type_char:
        return sizeof(char);
      case datatype::type_uint8:
        return sizeof(uint8_t);
      case datatype::type_uint16:
        return sizeof(uint16_t);
      case datatype::type_uint32:
        return sizeof(uint32_t);
      case datatype::type_uint64:
        return sizeof(uint64_t);
      case datatype::type_int8:
        return sizeof(int8_t);
      case datatype::type_int16:
        return sizeof(int16_t);
      case datatype::type_int32:
        return sizeof(int32_t);
      case datatype::type_int64:
        return sizeof(int64_t);
      case datatype::type_float:
        return sizeof(float);
      case datatype::type_double:
        return sizeof(double);
      case datatype::type_string:
        return sizeof(char);
      default:
        return 0;
    }
  }

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

  template<datatype T>
  static inline void write_type(std::FILE* f)
  {
    constexpr uint8_t type_byte = static_cast<uint8_t>(T);
    fwrite(&type_byte, sizeof(uint8_t), 1, f);
  }
};

}  // namespace binary_log
