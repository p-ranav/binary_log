#pragma once
#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

#include <binary_log/constant.hpp>
#include <binary_log/detail/args.hpp>

namespace binary_log
{
class packer
{
  std::FILE* m_log_file;
  std::FILE* m_index_file;
  std::FILE* m_runlength_file;

  // This buffer is buffering fwrite calls
  // to the log file.
  //
  // fwrite already has an internal buffer
  // but this buffer is used to avoid
  // multiple fwrite calls.
  constexpr static inline std::size_t buffer_size = 1024 * 1024;
  std::array<uint8_t, buffer_size> m_buffer;
  std::size_t m_buffer_index = 0;

  constexpr static inline std::size_t index_buffer_size = 1024;
  std::array<uint8_t, index_buffer_size> m_index_buffer;
  std::size_t m_index_buffer_index = 0;

  // Members for run-length encoding
  // of the log file.
  bool m_first_call = true;
  std::size_t m_runlength_index = 0;
  uint64_t m_current_runlength = 0;

  template<typename T>
  void buffer_or_write(T* input, std::size_t size)
  {
    if (m_buffer_index + size >= buffer_size) {
      fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
      m_buffer_index = 0;
    }

    const auto* byte_array = reinterpret_cast<const uint8_t*>(input);
    for (std::size_t i = 0; i < size; ++i)
      m_buffer[m_buffer_index++] = byte_array[i];
  }

  template<typename T>
  constexpr void buffer_or_write_index_file(T* input, std::size_t size)
  {
    if (m_index_buffer_index + size >= index_buffer_size) {
      fwrite(m_index_buffer.data(), sizeof(uint8_t), m_index_buffer_index, m_index_file);
      m_index_buffer_index = 0;
    }

    const auto* byte_array = reinterpret_cast<const uint8_t*>(input);
    for (std::size_t i = 0; i < size; ++i)
      m_index_buffer[m_index_buffer_index++] = byte_array[i];
  }

  // Variable-length encoding functions
  // for integers

  template <typename T> bool CHECK_BIT(T &value, uint8_t pos) {
    return ((value) & (1 << (pos)));
  }

  template <typename T> void SET_BIT(T &value, uint8_t pos) {
    value = value | 1 << pos;
  }

  template <typename T> void RESET_BIT(T &value, uint8_t pos) {
    value = value & ~(1 << pos);
  }

  template <std::size_t N>
  void append(uint8_t value, std::array<uint8_t, N> &container,
              std::size_t &index) {
    container[index++] = value;
  }

  template <typename int_t, typename Container>
  bool encode_varint_firstbyte_6(int_t &value, Container &output,
                                std::size_t &byte_index) {
    uint8_t octet = 0;
    if (value < 0) {
      value *= -1;
      SET_BIT(octet, 7);
    }
    // While more than 7 bits of data are left, occupy the last output byte
    // and set the next byte flag
    if (value > 63) {
      // Set the next byte flag
      octet |= ((uint8_t)(value & 63)) | 64;
      append(std::move(octet), output, byte_index);
      return true; // multibyte
    } else {
      octet |= ((uint8_t)(value & 63));
      append(std::move(octet), output, byte_index);
      return false; // no more bytes needed
    }
  }

  template <typename int_t, typename Container>
  void encode_varint_6(int_t value, Container &output, std::size_t &byte_index) {
    // While more than 7 bits of data are left, occupy the last output byte
    // and set the next byte flag
    while (value > 63) {
      // Set the next byte flag
      append(((uint8_t)(value & 63)) | 64, output, byte_index);
      // Remove the seven bits we just wrote
      value >>= 6;
    }
    append(((uint8_t)value) & 63, output, byte_index);
  }

  template <typename int_t, typename Container>
  void encode_varint_7(int_t value, Container &output, std::size_t &byte_index) {
    if (value < 0) {
      value *= 1;
    }
    // While more than 7 bits of data are left, occupy the last output byte
    // and set the next byte flag
    while (value > 127) {
      //|128: Set the next byte flag
      append(((uint8_t)(value & 127)) | 128, output, byte_index);
      // Remove the seven bits we just wrote
      value >>= 7;
    }
    append(((uint8_t)value) & 127, output, byte_index);
  }

  // Unsigned integer variable-length encoding functions
  template <typename int_t, typename Container>
  typename std::enable_if<std::is_integral_v<int_t> && !std::is_signed_v<int_t>,
                          void>::type
  encode_varint(int_t value, Container &output, std::size_t &byte_index) {
    encode_varint_7<int_t>(value, output, byte_index);
  }

  // Signed integer variable-length encoding functions
  template <typename int_t, typename Container>
  typename std::enable_if<std::is_integral_v<int_t> && std::is_signed_v<int_t>,
                          void>::type
  encode_varint(int_t value, Container &output, std::size_t &byte_index) {
    // first octet
    if (encode_varint_firstbyte_6<int_t>(value, output, byte_index)) {
      // rest of the octets
      encode_varint_7<int_t>(value, output, byte_index);
    }
  }

public:
  packer(const char* path)
  {
    // Create the log file
    // All the log contents go here
    m_log_file = fopen(path, "wb");
    if (m_log_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }

    // No fwrite buffering
    setvbuf(m_log_file, nullptr, _IONBF, 0);

    // Create the index file
    std::string index_file_path = std::string {path} + ".index";
    m_index_file = fopen(index_file_path.data(), "wb");
    if (m_index_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }

    // No fwrite buffering
    setvbuf(m_index_file, nullptr, _IONBF, 0);

    // Create the runlength file
    std::string runlength_file_path = std::string {path} + ".runlength";
    m_runlength_file = fopen(runlength_file_path.data(), "wb");
    if (m_runlength_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }
  }

  ~packer()
  {
    flush();
    fclose(m_log_file);
    fclose(m_index_file);
    fclose(m_runlength_file);
  }

  void flush_log_file()
  {
    if (m_buffer_index) {
      fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
      m_buffer_index = 0;
    }
    fflush(m_log_file);
  }

  void flush_index_file()
  {
    if (m_index_buffer_index) {
      fwrite(m_index_buffer.data(), sizeof(uint8_t), m_index_buffer_index, m_index_file);
      m_index_buffer_index = 0;
    }
    fflush(m_index_file);
  }

  void flush_runlength_file()
  {
    write_current_runlength_to_runlength_file();
    fflush(m_runlength_file);
  }

  void flush()
  {
    flush_index_file();
    flush_log_file();
    flush_runlength_file();
  }

  template<typename T>
  inline void write_arg_value_to_log_file(T&& input) = delete;

  inline void write_arg_value_to_log_file(const char* input)
  {
    const uint8_t size = static_cast<uint8_t>(std::strlen(input));
    buffer_or_write(&size, sizeof(uint8_t));
    buffer_or_write(input, size);
  }

  inline void write_arg_value_to_log_file(char input)
  {
    buffer_or_write(&input, sizeof(char));
  }

  inline void write_arg_value_to_log_file(bool input)
  {
    buffer_or_write(&input, sizeof(bool));
  }

  inline void write_arg_value_to_log_file(uint8_t input)
  {
    constexpr uint8_t bytes = 1;
    buffer_or_write(&bytes, sizeof(uint8_t));
    buffer_or_write(&input, sizeof(uint8_t));
  }

  inline void write_arg_value_to_log_file(uint16_t input)
  {
    if (input <= std::numeric_limits<uint8_t>::max()) {
      uint8_t value = static_cast<uint8_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 2;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(uint16_t));
    }
  }

  inline void write_arg_value_to_log_file(uint32_t input)
  {
    if (input <= std::numeric_limits<uint16_t>::max()) {
      uint16_t value = static_cast<uint16_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 4;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(uint32_t));
    }
  }

  inline void write_arg_value_to_log_file(uint64_t input)
  {
    if (input <= std::numeric_limits<uint32_t>::max()) {
      uint32_t value = static_cast<uint32_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 8;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(uint64_t));
    }
  }

  inline void write_arg_value_to_log_file(int8_t input)
  {
    constexpr uint8_t bytes = 1;
    buffer_or_write(&bytes, sizeof(uint8_t));
    buffer_or_write(&input, sizeof(int8_t));
  }

  inline void write_arg_value_to_log_file(int16_t input)
  {
    if (input <= std::numeric_limits<int8_t>::max()
        && input >= std::numeric_limits<int8_t>::min())
    {
      int8_t value = static_cast<int8_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 2;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(int16_t));
    }
  }

  inline void write_arg_value_to_log_file(int32_t input)
  {
    if (input <= std::numeric_limits<int16_t>::max()
        && input >= std::numeric_limits<int16_t>::min())
    {
      int16_t value = static_cast<int16_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 4;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(int32_t));
    }
  }

  inline void write_arg_value_to_log_file(int64_t input)
  {
    if (input <= std::numeric_limits<int32_t>::max()
        && input >= std::numeric_limits<int32_t>::min())
    {
      int32_t value = static_cast<int32_t>(input);
      write_arg_value_to_log_file(value);
    } else {
      constexpr uint8_t bytes = 8;
      buffer_or_write(&bytes, sizeof(uint8_t));
      buffer_or_write(&input, sizeof(int64_t));
    }
  }

  inline void write_arg_value_to_log_file(float input)
  {
    buffer_or_write(&input, sizeof(float));
  }

  inline void write_arg_value_to_log_file(double input)
  {
    buffer_or_write(&input, sizeof(double));
  }

  template<typename T>
  requires is_string_type<T> inline void write_arg_value_to_log_file(T&& input)
  {
    const uint8_t size = static_cast<uint8_t>(input.size());
    buffer_or_write(&size, sizeof(uint8_t));
    buffer_or_write(input.data(), size);
  }

  template<typename T>
  constexpr inline void pack_arg(T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      write_arg_value_to_log_file(std::forward<T>(input));
    }
  }

  inline void write_current_runlength_to_runlength_file()
  {
    if (m_current_runlength > 1) {
      const uint8_t index = static_cast<uint8_t>(m_runlength_index);
      fwrite(&index, sizeof(uint8_t), 1, m_runlength_file);

      // Write runlength to file
      // Perform integer compression
      if (m_current_runlength <= std::numeric_limits<uint8_t>::max()) {
        uint8_t value = static_cast<uint8_t>(m_current_runlength);
        constexpr uint8_t bytes = 1;
        fwrite(&bytes, sizeof(uint8_t), 1, m_runlength_file);
        fwrite(&value, sizeof(uint8_t), 1, m_runlength_file);
      } else if (m_current_runlength <= std::numeric_limits<uint16_t>::max()) {
        uint16_t value = static_cast<uint16_t>(m_current_runlength);
        constexpr uint8_t bytes = 2;
        fwrite(&bytes, sizeof(uint8_t), 1, m_runlength_file);
        fwrite(&value, sizeof(uint16_t), 1, m_runlength_file);
      } else if (m_current_runlength <= std::numeric_limits<uint32_t>::max()) {
        uint32_t value = static_cast<uint32_t>(m_current_runlength);
        constexpr uint8_t bytes = 4;
        fwrite(&bytes, sizeof(uint8_t), 1, m_runlength_file);
        fwrite(&value, sizeof(uint32_t), 1, m_runlength_file);
      } else {
        uint64_t value = static_cast<uint64_t>(m_current_runlength);
        constexpr uint8_t bytes = 8;
        fwrite(&bytes, sizeof(uint8_t), 1, m_runlength_file);
        fwrite(&value, sizeof(uint64_t), 1, m_runlength_file);
      }

      m_current_runlength = 0;
    }
  }

  constexpr inline void pack_format_string_index(uint8_t& index)
  {
    // Evaluate this index
    //
    // If index is the same as the m_runlength_index
    // but m_current_runlength is 0, write it and increment m_current_runlength
    // else, no need to write it, just update the runlength
    //
    // If the index is different from m_runlength_index
    // then, write (m_runlength_index + m_current_runlength)
    // to the m_runlength_file and write the new index
    // to the logfile

    if (m_runlength_index == 0 && index == 0) {
      // First index
      if (m_current_runlength == 0) {
        // First call
        // Write index to log file
        buffer_or_write(&index, sizeof(uint8_t));
        m_current_runlength++;
      } else if (m_current_runlength >= 1) {
        m_current_runlength++;
      }
    } else {
      // Not first index
      if (m_runlength_index == index) {
        // No change to index
        m_current_runlength++;
      } else {
        // Write current runlength to file
        write_current_runlength_to_runlength_file();

        // Write index to log file
        buffer_or_write(&index, sizeof(uint8_t));
        m_current_runlength = 1;
        m_runlength_index = index;
      }
    }
  }

  template<class... Args>
  constexpr inline void update_log_file(Args&&... args)
  {
    ((void)pack_arg(std::forward<Args>(args)), ...);
  }

  template<fmt_arg_type T>
  constexpr inline void write_arg_type()
  {
    constexpr uint8_t type_byte = static_cast<uint8_t>(T);
    buffer_or_write_index_file(&type_byte, sizeof(uint8_t));
  }

  template<typename T>
  constexpr inline void save_arg_type()
  {
    using type = typename std::decay<T>::type;

    if constexpr (is_specialization<type, constant> {}) {
      // This is a constant
      using inner_type = typename T::type;
      write_arg_type<binary_log::get_arg_type<inner_type>()>();
    } else {
      write_arg_type<binary_log::get_arg_type<type>()>();
    }
  }

  template<typename T>
  inline void write_arg_value_to_index_file(T&& input) = delete;

  inline void write_arg_value_to_index_file(const char* input)
  {
    const uint8_t size = static_cast<uint8_t>(std::strlen(input));
    buffer_or_write_index_file(&size, sizeof(uint8_t));
    buffer_or_write_index_file(input, size);
  }

  template<typename T>
  requires is_numeric_type<T> inline void write_arg_value_to_index_file(
      T&& input)
  {
    buffer_or_write_index_file(&input, sizeof(T));
  }

  template<typename T>
  requires is_string_type<T> inline void write_arg_value_to_index_file(
      T&& input)
  {
    const uint8_t size = static_cast<uint8_t>(input.size());
    buffer_or_write_index_file(&size, sizeof(uint8_t));
    buffer_or_write_index_file(input.data(), input.size());
  }

  template<typename T>
  constexpr inline void save_arg_constness(T&& input)
  {
    if constexpr (!is_specialization<T, constant> {}) {
      constexpr bool is_constant = false;
      buffer_or_write_index_file(&is_constant, sizeof(bool));
    } else {
      constexpr bool is_constant = true;
      buffer_or_write_index_file(&is_constant, sizeof(bool));
      write_arg_value_to_index_file(input.value);
    }
  }

  template<class... Args>
  constexpr inline void update_index_file(Args&&... args)
  {
    ((void)save_arg_type<Args>(), ...);
    ((void)save_arg_constness(std::forward<Args>(args)), ...);
  }

  template <const char* format_string>
  constexpr inline void write_format_string_to_index_file()
  {
    constexpr uint8_t length = strlen(format_string);
    buffer_or_write_index_file(&length, sizeof(uint8_t));
    buffer_or_write_index_file(format_string, length);

    // Initialize the runlength member variables
    // This might be the first log call on this logger
    if (m_first_call) {
      m_runlength_index = 0;
      m_current_runlength = 0;
      m_first_call = false;
    }
  }

  constexpr inline void write_num_args_to_index_file(const uint8_t& num_args)
  {
    buffer_or_write_index_file(&num_args, sizeof(uint8_t));
  }
};

}  // namespace binary_log
