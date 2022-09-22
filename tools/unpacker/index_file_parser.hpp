#pragma once
#include <string_view>

#include <binary_log/detail/args.hpp>
#include <mio.hpp>

namespace binary_log
{
class index_file_parser
{
  mio::mmap_source m_mmap;
  const char* m_buffer {nullptr};
  size_t m_buffer_size {0};

  std::size_t m_index {0};  // into m_buffer

public:
  struct arg
  {
    fmt_arg_type type;
    bool is_constant;

    // if constant, this will have the static data
    std::string_view arg_data;
  };

  struct index_entry
  {
    std::string_view format_string;
    std::vector<arg> args;
  };

private:
  std::vector<index_entry> m_entries;

  uint8_t next_byte()
  {
    return *reinterpret_cast<const uint8_t*>(&m_buffer[m_index++]);
  }

  uint16_t next_two_bytes()
  {
    const uint16_t* bytes = reinterpret_cast<const uint16_t*>(&m_buffer[m_index]);
    m_index += 2;
    return *bytes;
  }

  void parse_entry()
  {
    index_entry entry;
    // This is the start of an entry.

    // First, parse the size of the format string
    // This is assumed to be 1 byte.
    const std::size_t format_string_size = next_two_bytes();

    // Next, parse the format string
    std::string_view format_string(m_buffer + m_index, format_string_size);
    entry.format_string = format_string;

    // Next, parse the number of arguments
    m_index += format_string_size;
    const std::size_t num_args = next_byte();

    if (num_args > 0) {
      // Parse the arg type of each arg
      // This is 1 byte * num_args
      std::vector<fmt_arg_type> arg_types;
      for (std::size_t i = 0; i < num_args; ++i) {
        uint8_t arg_type = next_byte();
        fmt_arg_type type = static_cast<fmt_arg_type>(arg_type);
        arg_types.push_back(type);
      }

      // The next set of bytes will have the format
      // <is_constant> <arg-value>? <is_constant> <arg-value>? ...
      for (std::size_t i = 0; i < num_args; ++i) {
        arg arg;
        arg.type = arg_types[i];
        arg.is_constant = m_buffer[m_index++];

        if (arg.is_constant) {
          // the next byte will be the value
          // size is determined by the type of the arg
          std::size_t size = 0;
          if (arg.type == fmt_arg_type::type_string) {
            // the next byte will be the size of the string
            size = next_two_bytes();
          } else {
            // size is determined by the type of the arg
            size = sizeof_arg_type(arg.type);
          }
          // the next bytes will be the data
          arg.arg_data = std::string_view(m_buffer + m_index, size);
          m_index += size;
        }
        entry.args.push_back(arg);
      }
    }

    m_entries.push_back(entry);
  }

public:
  index_file_parser(const char* path)
  {
    m_mmap = mio::mmap_source(path);
    if (!m_mmap.is_open() || !m_mmap.is_mapped()) {
      throw std::runtime_error("Could not open index file");
    }

    m_buffer = m_mmap.data();
    m_buffer_size = m_mmap.mapped_length();
  }

  std::vector<index_entry> parse()
  {
    while (m_index < m_buffer_size) {
      parse_entry();
    }
    return m_entries;
  }
};

}  // namespace binary_log