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
    type_string_view,
    type_string,
  };

  template<typename T>
  static inline std::array<uint8_t, sizeof(T)> to_byte_array(const T& input)
  {
    std::array<uint8_t, sizeof(T)> ret;
    std::memcpy(ret.data(), &input, sizeof(T));
    return ret;
  }

  template<typename T>
  static inline void write_type(std::FILE* f, T type)
  {
    uint8_t type_byte = static_cast<uint8_t>(type);
    fwrite(&type_byte, sizeof(uint8_t), 1, f);
  }

  template<typename T>
  static inline void pack(std::FILE* f, const T& input) = delete;

  static inline void pack(std::FILE* f, const char& input)
  {
    write_type(f, datatype::type_char);
    fwrite(&input, sizeof(char), 1, f);
  }

  static inline void pack(std::FILE* f, const uint8_t& input)
  {
    write_type(f, datatype::type_uint8);
    fwrite(&input, sizeof(uint8_t), 1, f);
  }

  static inline void pack(std::FILE* f, const uint16_t& input)
  {
    if (input < std::numeric_limits<uint8_t>::max()) {
      write_type(f, datatype::type_uint8);
      fwrite(&input, sizeof(uint8_t), 1, f);
    } else {
      write_type(f, datatype::type_uint16);
      fwrite(&input, sizeof(uint16_t), 1, f);
    }
  }

  static inline void pack(std::FILE* f, const uint32_t& input)
  {
    if (input < std::numeric_limits<uint8_t>::max()) {
      write_type(f, datatype::type_uint8);
      fwrite(&input, sizeof(uint8_t), 1, f);
    } else if (input < std::numeric_limits<uint16_t>::max()) {
      write_type(f, datatype::type_uint16);
      fwrite(&input, sizeof(uint16_t), 1, f);
    } else {
      write_type(f, datatype::type_uint32);
      fwrite(&input, sizeof(uint32_t), 1, f);
    }
  }

  static inline void pack(std::FILE* f, const uint64_t& input)
  {
    if (input < std::numeric_limits<uint8_t>::max()) {
      write_type(f, datatype::type_uint8);
      fwrite(&input, sizeof(uint8_t), 1, f);
    } else if (input < std::numeric_limits<uint16_t>::max()) {
      write_type(f, datatype::type_uint16);
      fwrite(&input, sizeof(uint16_t), 1, f);
    } else if (input < std::numeric_limits<uint32_t>::max()) {
      write_type(f, datatype::type_uint32);
      fwrite(&input, sizeof(uint32_t), 1, f);
    } else {
      write_type(f, datatype::type_uint64);
      fwrite(&input, sizeof(uint64_t), 1, f);
    }
  }

  static inline void pack(std::FILE* f, const int8_t& input)
  {
    write_type(f, datatype::type_int8);
    fwrite(&input, sizeof(int8_t), 1, f);
  }

  static inline void pack(std::FILE* f, const int16_t& input)
  {
    if (input < std::numeric_limits<int8_t>::min()) {
      write_type(f, datatype::type_int8);
      fwrite(&input, sizeof(int8_t), 1, f);
    } else {
      write_type(f, datatype::type_int16);
      fwrite(&input, sizeof(int16_t), 1, f);
    }
  }

  static inline void pack(std::FILE* f, const int32_t& input)
  {
    if (input < std::numeric_limits<int8_t>::min()) {
      write_type(f, datatype::type_int8);
      fwrite(&input, sizeof(int8_t), 1, f);
    } else if (input < std::numeric_limits<int16_t>::min()) {
      write_type(f, datatype::type_int16);
      fwrite(&input, sizeof(int16_t), 1, f);
    } else {
      write_type(f, datatype::type_int32);
      fwrite(&input, sizeof(int32_t), 1, f);
    }
  }

  static inline void pack(std::FILE* f, const int64_t& input)
  {
    if (input < std::numeric_limits<int8_t>::min()) {
      write_type(f, datatype::type_int8);
      fwrite(&input, sizeof(int8_t), 1, f);
    } else if (input < std::numeric_limits<int16_t>::min()) {
      write_type(f, datatype::type_int16);
      fwrite(&input, sizeof(int16_t), 1, f);
    } else if (input < std::numeric_limits<int32_t>::min()) {
      write_type(f, datatype::type_int32);
      fwrite(&input, sizeof(int32_t), 1, f);
    } else {
      write_type(f, datatype::type_int64);
      fwrite(&input, sizeof(int64_t), 1, f);
    }
  }

  static inline void pack(std::FILE* f, const float& input)
  {
    write_type(f, datatype::type_float);
    fwrite(&input, sizeof(float), 1, f);
  }

  static inline void pack(std::FILE* f, const double& input)
  {
    write_type(f, datatype::type_double);
    fwrite(&input, sizeof(double), 1, f);
  }

  // static inline void pack(std::FILE* f, const std::string_view& input)
  // {
  // 	write_type(f, datatype::type_string);
  // 	pack<uint32_t>(f, input.size());
  // 	fwrite(input.data(), sizeof(char), input.size(), f);
  // }

  // static inline void pack(std::FILE* f, const std::string& input)
  // {
  // 	write_type(f, datatype::type_string);
  // 	pack<uint32_t>(f, input.size());
  // 	fwrite(input.data(), sizeof(char), input.size(), f);
  // }
};

}  // namespace binary_log