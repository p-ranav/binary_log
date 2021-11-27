#pragma once

namespace binary_log
{
constexpr static inline uint8_t string_length(const char* str)
{
  return *str ? 1 + string_length(str + 1) : 0;
}

constexpr static inline bool strings_equal(char const* a, char const* b)
{
  return *a == *b && (*a == '\0' || strings_equal(a + 1, b + 1));
}

}  // namespace binary_log