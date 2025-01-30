#pragma once
#include <array>
#include <cstring>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>

#include <binary_log/constant.hpp>
#include <binary_log/detail/args.hpp>

namespace binary_log
{
template<size_t log_buffer_size= 1 * 1024 * 1024, size_t index_buffer_size = 32, size_t runlength_buffer_size = 32>
class packer
{
  std::filesystem::path m_path;
  std::FILE* m_log_file;
  std::FILE* m_index_file;
  std::FILE* m_runlength_file;

  // This buffer is buffering fwrite calls
  // to the log file.
  //
  // fwrite already has an internal buffer
  // but this buffer is used to avoid
  // multiple fwrite calls.
  std::array<uint8_t, log_buffer_size> m_buffer;
  std::size_t m_buffer_index = 0;

  std::array<uint8_t, index_buffer_size> m_index_buffer;
  std::size_t m_index_buffer_index = 0;

  // Members for run-length encoding
  // of the log file.
  bool m_first_call = true;
  std::array<uint8_t, runlength_buffer_size> m_runlength_buffer;
  std::size_t m_runlength_buffer_index = 0;
  std::size_t m_runlength_index = 0;
  uint64_t m_current_runlength = 0;

  template<typename T, std::size_t size>
  void buffer_or_write(T* input)
  {
    buffer_or_write(input, size);
  }

  template<typename T>
  void buffer_or_write(T* input, std::size_t size)
  {
    std::size_t bytes_left = size;
    auto* byte_array = reinterpret_cast<const uint8_t*>(input);
    while (bytes_left) {
      if (m_buffer_index + bytes_left >= log_buffer_size) {
        fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
        m_buffer_index = 0;
      }

      std::size_t num_bytes_to_copy = std::min(bytes_left, log_buffer_size);
      std::copy_n(byte_array, num_bytes_to_copy, &m_buffer[m_buffer_index]);
      byte_array += num_bytes_to_copy;
      m_buffer_index += num_bytes_to_copy;
      bytes_left -= num_bytes_to_copy;
    }
  }

  template<typename T>
  constexpr void buffer_or_write_index_file(T* input, std::size_t size)
  {
    std::size_t bytes_left = size;
    const auto* byte_array = reinterpret_cast<const uint8_t*>(input);
    while (bytes_left) {
      if (m_index_buffer_index + bytes_left >= index_buffer_size) {
        fwrite(m_index_buffer.data(),
               sizeof(uint8_t),
               m_index_buffer_index,
               m_index_file);
        m_index_buffer_index = 0;
      }

      std::size_t num_bytes_to_copy = std::min(bytes_left, index_buffer_size-1);
      std::copy_n(byte_array, num_bytes_to_copy, &m_index_buffer[m_index_buffer_index]);
      byte_array += num_bytes_to_copy;
      m_index_buffer_index += num_bytes_to_copy;
      bytes_left -= num_bytes_to_copy;
    }
  }

public:
  packer(const std::filesystem::path& path) : m_path(path)
  {
    // Create the log file
    // All the log contents go here
    m_log_file = fopen(get_log_path().c_str(), "wb");
    if (m_log_file == nullptr) {
#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711L
      throw std::invalid_argument("fopen failed");
#else
      abort();
#endif
    }

    // No fwrite buffering
    setvbuf(m_log_file, nullptr, _IONBF, 0);

    // Create the index file
    m_index_file = fopen(get_index_path().c_str(), "wb");
    if (m_index_file == nullptr) {
#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711L
      throw std::invalid_argument("fopen failed");
#else
      abort();
#endif
    }

    // No fwrite buffering
    setvbuf(m_index_file, nullptr, _IONBF, 0);

    // Create the runlength file
    m_runlength_file = fopen(get_runlength_path().c_str(), "wb");
    if (m_runlength_file == nullptr) {
#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711L
      throw std::invalid_argument("fopen failed");
#else
      abort();
#endif
    }

    m_runlength_index = 0;
    m_current_runlength = 0;
  }

  packer(const char* path) : packer(std::filesystem::path {path})
  {
  }

  ~packer()
  {
    flush();
    fclose(m_log_file);
    fclose(m_index_file);
    fclose(m_runlength_file);
  }

  std::filesystem::path get_log_path() const
  {
    return m_path;
  }

  std::filesystem::path get_index_path() const
  {
    std::filesystem::path index_file_path = m_path;
    index_file_path.replace_extension(m_path.extension().string() + ".index");
    return index_file_path;
  }

  std::filesystem::path get_runlength_path() const
  {
    std::filesystem::path runlength_file_path = m_path;
    runlength_file_path.replace_extension(m_path.extension().string() + ".runlength");
    return runlength_file_path;
  }

  void flush_log_file()
  {
    if (m_log_file == nullptr) {
      return;
    }
    fwrite(m_buffer.data(), sizeof(uint8_t), m_buffer_index, m_log_file);
    m_buffer_index = 0;
    fflush(m_log_file);
  }

  void flush_index_file()
  {
    if (m_index_file == nullptr) {
      return;
    }
    fwrite(m_index_buffer.data(),
           sizeof(uint8_t),
           m_index_buffer_index,
           m_index_file);
    m_index_buffer_index = 0;
    fflush(m_index_file);
  }

  void flush_runlength_file()
  {
    if (m_runlength_file == nullptr) {
      return;
    }
    write_current_runlength_to_runlength_file();
    fwrite(m_runlength_buffer.data(),
           sizeof(uint8_t),
           m_runlength_buffer_index,
           m_runlength_file);
    m_runlength_buffer_index = 0;
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
    uint16_t size = static_cast<uint16_t>(std::strlen(input));
    buffer_or_write<uint16_t, sizeof(uint16_t)>(&size);
    buffer_or_write(input, size);
  }

  inline void write_arg_value_to_log_file(char input)
  {
    buffer_or_write<char, sizeof(char)>(&input);
  }

  inline void write_arg_value_to_log_file(bool input)
  {
    buffer_or_write<bool, sizeof(bool)>(&input);
  }

  inline void write_arg_value_to_log_file(uint8_t input)
  {
    buffer_or_write<uint8_t, sizeof(uint8_t)>(&input);
  }

  inline void write_arg_value_to_log_file(uint16_t input)
  {
    buffer_or_write<uint16_t, sizeof(uint16_t)>(&input);
  }

  inline void write_arg_value_to_log_file(uint32_t input)
  {
    buffer_or_write<uint32_t, sizeof(uint32_t)>(&input);
  }

  inline void write_arg_value_to_log_file(uint64_t input)
  {
    buffer_or_write<uint64_t, sizeof(uint64_t)>(&input);
  }

  template<typename U>
  inline void write_arg_value_to_log_file(U &input) requires (std::is_same_v<U, std::size_t>)
  {
    buffer_or_write<U, sizeof(U)>(&input);
  }

  inline void write_arg_value_to_log_file(int8_t input)
  {
    buffer_or_write<int8_t, sizeof(int8_t)>(&input);
  }

  inline void write_arg_value_to_log_file(int16_t input)
  {
    buffer_or_write<int16_t, sizeof(int16_t)>(&input);
  }

  inline void write_arg_value_to_log_file(int32_t input)
  {
    buffer_or_write<int32_t, sizeof(int32_t)>(&input);
  }

  inline void write_arg_value_to_log_file(int64_t input)
  {
    buffer_or_write<int64_t, sizeof(int64_t)>(&input);
  }

  inline void write_arg_value_to_log_file(float input)
  {
    buffer_or_write<float, sizeof(float)>(&input);
  }

  inline void write_arg_value_to_log_file(double input)
  {
    buffer_or_write<double, sizeof(double)>(&input);
  }

  template<typename T>
  requires is_string_type<T> inline void write_arg_value_to_log_file(T&& input)
  {
    uint16_t size = static_cast<uint16_t>(input.size());
    buffer_or_write<uint16_t, sizeof(uint16_t)>(&size);
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
      size_t size = sizeof(uint16_t) + sizeof(uint64_t);
      size_t bytes_left = size;
      // make the bytes we'll write to the runlength file
      uint8_t bytes[size];
      // fill the bytes
      std::memcpy(&bytes[0], &m_runlength_index, sizeof(uint16_t));
      std::memcpy(&bytes[sizeof(uint16_t)], &m_current_runlength, sizeof(uint64_t));
      // write the bytes
      while (bytes_left) {
        if (m_runlength_buffer_index + bytes_left >= runlength_buffer_size) {
          fwrite(m_runlength_buffer.data(),
                 sizeof(uint8_t),
                 m_runlength_buffer_index,
                 m_runlength_file);
          m_runlength_buffer_index = 0;
        }

        std::size_t num_bytes_to_copy = std::min(bytes_left, runlength_buffer_size);
        std::copy_n(bytes, num_bytes_to_copy, &m_runlength_buffer[m_runlength_buffer_index]);
        m_runlength_buffer_index += num_bytes_to_copy;
        bytes_left -= num_bytes_to_copy;
      }
      // reset the runlength
      m_current_runlength = 0;
    }
  }

  constexpr inline void pack_format_string_index(uint16_t index)
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
        buffer_or_write<uint16_t, sizeof(uint16_t)>(&index);
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
        buffer_or_write<uint16_t, sizeof(uint16_t)>(&index);
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
    const uint16_t size = static_cast<uint16_t>(std::strlen(input));
    buffer_or_write_index_file(&size, sizeof(uint16_t));
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
    const uint16_t size = static_cast<uint16_t>(input.size());
    buffer_or_write_index_file(&size, sizeof(uint16_t));
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

  template<const char* format_string>
  void write_format_string_to_index_file()
  {
    const uint16_t length = static_cast<uint16_t>(std::strlen(format_string));
    buffer_or_write_index_file(&length, sizeof(uint16_t));
    buffer_or_write_index_file(format_string, length);
  }

  constexpr inline void write_num_args_to_index_file(const uint8_t& num_args)
  {
    buffer_or_write_index_file(&num_args, sizeof(uint8_t));
  }
};

}  // namespace binary_log
