#pragma once
#include <cstdint>

namespace binary_log {

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

template<fmt_arg_type T>
static inline void write_arg_type(std::FILE* f)
{
  constexpr uint8_t type_byte = static_cast<uint8_t>(T);
  fwrite(&type_byte, sizeof(uint8_t), 1, f);
}

}