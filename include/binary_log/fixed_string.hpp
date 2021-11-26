#pragma once

namespace binary_log
{
template<unsigned N>
struct fixed_string
{
  char buf[N + 1] {};
  constexpr fixed_string(char const* s)
  {
    for (unsigned i = 0; i != N; ++i)
      buf[i] = s[i];
  }
  constexpr operator char const *() const
  {
    return buf;
  }
};
template<unsigned N>
fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

}  // namespace binary_log