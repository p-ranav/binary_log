#pragma once
#include <iostream>
#include <string_view>
#include <vector>

#include <index_file_parser.hpp>
#include <mio.hpp>

namespace binary_log
{
class log_file_parser
{
  mio::mmap_source m_mmap;
  const char* m_buffer {nullptr};
  size_t m_buffer_size {0};

  std::size_t m_index {0};  // into m_buffer

  struct arg
  {
    fmt_arg_type type;
    std::size_t size;
    std::string_view value;
  };

  struct log_entry
  {
    std::size_t format_string_index;
    std::vector<arg> args;
  };

  uint8_t next_byte()
  {
    return *reinterpret_cast<const uint8_t*>(&m_buffer[m_index++]);
  }

  log_entry parse_entry(
      const std::vector<index_file_parser::index_entry>& index_table)
  {
    log_entry entry;

    // First parse the index into the index table
    std::size_t index = next_byte();

    entry.format_string_index = index;

    auto index_entry = index_table[index];

    // Now we know the format string and the number of arguments
    // along with the type of each argument to be parsed

    for (const auto& arg_info : index_entry.args) {
      arg new_arg;
      new_arg.type = arg_info.type;

      if (arg_info.is_constant) {
        // The argument value is in the index table
        new_arg.value = arg_info.arg_data;
      } else {
        // argument is not constant
        // need to parse it from the log file
        //
        // first find out how many bytes the argument takes
        // then parse the argument

        std::size_t size = 0;
        if (arg_info.type == fmt_arg_type::type_string
            || arg_info.type == fmt_arg_type::type_uint8
            || arg_info.type == fmt_arg_type::type_uint16
            || arg_info.type == fmt_arg_type::type_uint32
            || arg_info.type == fmt_arg_type::type_uint64
            || arg_info.type == fmt_arg_type::type_int8
            || arg_info.type == fmt_arg_type::type_int16
            || arg_info.type == fmt_arg_type::type_int32
            || arg_info.type == fmt_arg_type::type_int64)
        {
          // Next byte is the size
          // For strings, this is the string length
          // For integers, this is the real size of the integer
          // - binary_log likes to shrink integers to smaller sizes when
          // possible
          size = next_byte();
        } else {
          // size if the size of the type
          size = sizeof_arg_type(arg_info.type);
        }
        // the next bytes will be the data
        new_arg.size = size;
        new_arg.value = std::string_view(
            reinterpret_cast<const char*>(&m_buffer[m_index]), size);
        m_index += size;
      }

      entry.args.push_back(new_arg);
    }

    return entry;
  };

public:
  log_file_parser(const char* path)
  {
    m_mmap = mio::mmap_source(path);
    if (!m_mmap.is_open() || !m_mmap.is_mapped()) {
      throw std::runtime_error("Could not open log file");
    }

    m_buffer = m_mmap.data();
    m_buffer_size = m_mmap.mapped_length();
  }

  std::vector<log_entry> parse(
      const std::vector<index_file_parser::index_entry>& index_table)
  {
    std::vector<log_entry> entries;

    while (m_index < m_buffer_size) {
      entries.push_back(parse_entry(index_table));
    }

    return entries;
  }
};
}  // namespace binary_log
