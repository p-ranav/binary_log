#pragma once

namespace binary_log
{

template<typename T>
struct constant
{
  using type = T;
  T value;
  constexpr constant(T v)
      : value(v)
  {
  }
};

}  // namespace binary_log