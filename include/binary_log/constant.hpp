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

}  // namespace binary_log