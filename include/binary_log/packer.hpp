#pragma once
#include <array>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>

namespace binary_log
{
struct packer
{
  enum class datatype
  {
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
  static inline void pack_data(std::FILE* f, const T& input) = delete;

  constexpr static inline void pack_data(std::FILE* f, const char& input)
  {
    fwrite(&input, sizeof(char), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const uint8_t& input)
  {
    fwrite(&input, sizeof(uint8_t), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const uint16_t& input)
  {
    fwrite(&input, sizeof(uint16_t), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const uint32_t& input)
  {
    fwrite(&input, sizeof(uint32_t), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const uint64_t& input)
  {
    fwrite(&input, sizeof(uint64_t), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const int8_t& input)
  {
    fwrite(&input, sizeof(int8_t), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const int16_t& input)
  {
    fwrite(&input, sizeof(int16_t), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const int32_t& input)
  {
    fwrite(&input, sizeof(int32_t), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const int64_t& input)
  {
    fwrite(&input, sizeof(int64_t), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const float& input)
  {
    fwrite(&input, sizeof(float), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const double& input)
  {
    fwrite(&input, sizeof(double), 1, f);
  }

  constexpr static inline void pack_data(std::FILE* f, const char* input)
  {
    pack_data(f, static_cast<uint8_t>(std::strlen(input)));
    fwrite(input, sizeof(char), std::strlen(input), f);
  }

  static inline void pack_data(std::FILE* f, const std::string& input)
  {
    pack_data(f, static_cast<uint8_t>(input.size()));
    fwrite(input.data(), sizeof(char), input.size(), f);
  }

  constexpr static inline void pack_data(std::FILE* f,
                                         const std::string_view& input)
  {
    pack_data(f, static_cast<uint8_t>(input.size()));
    fwrite(input.data(), sizeof(char), input.size(), f);
  }

  template<datatype T>
  constexpr static inline void write_type(std::FILE* f)
  {
    constexpr uint8_t type_byte = static_cast<uint8_t>(T);
    fwrite(&type_byte, sizeof(uint8_t), 1, f);
  }
};

}  // namespace binary_log