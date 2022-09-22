#pragma once
#include <cstdint>

#include <binary_log/constant.hpp>
#include <binary_log/detail/concepts.hpp>
#include <binary_log/detail/is_specialization.hpp>

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