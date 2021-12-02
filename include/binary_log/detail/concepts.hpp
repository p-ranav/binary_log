#pragma once
#include <concepts>

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