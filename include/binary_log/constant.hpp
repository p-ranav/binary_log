#pragma once

namespace binary_log
{
template<class T, template<class...> class Template>
struct is_specialization : std::false_type
{
};

template<template<class...> class Template, class... Args>
struct is_specialization<Template<Args...>, Template> : std::true_type
{
};

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

constexpr static inline bool all_args_are_constants()
{
  return true;
}

template<class T, class... Ts>
constexpr static inline bool all_args_are_constants(T&&, Ts&&... rest)
{
  if constexpr (is_specialization<T, constant> {}) {
    return all_args_are_constants(std::forward<Ts>(rest)...);
  } else {
    return false;
  }
}

}  // namespace binary_log