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
struct constant_wrapper
{
  T value;
  constexpr constant_wrapper(T v)
      : value(v)
  {
  }
};

}  // namespace binary_log