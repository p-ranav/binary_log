#pragma once
#include <iostream>
#include <string_view>
#include <vector>

#include <index_file_parser.hpp>
#include <mio.hpp>

#define FMT_HEADER_ONLY
#include <fmt/args.h>
#include <fmt/format.h>

namespace binary_log
{
class log_file_parser
{
  // Members for parsing the log file
  mio::mmap_source m_logfile_mmap;
  const char* m_logfile_buffer {nullptr};
  size_t m_logfile_buffer_size {0};
  std::size_t m_logfile_index {0};  // into m_logfile_buffer

  // Members for parsing the runlength file
  mio::mmap_source m_runlengthfile_mmap;
  const char* m_runlengthfile_buffer {nullptr};
  size_t m_runlengthfile_buffer_size {0};
  std::size_t m_runlengthfile_index {0};  // into m_runlength_buffer

  struct arg
  {
    fmt_arg_type type;
    std::size_t size;
    std::string_view value;
  };

  uint8_t next_byte_in_log_file()
  {
    return *reinterpret_cast<const uint8_t*>(
        &m_logfile_buffer[m_logfile_index++]);
  }

  uint8_t current_byte_in_log_file()
  {
    return *reinterpret_cast<const uint8_t*>(
        &m_logfile_buffer[m_logfile_index]);
  }

  uint8_t next_byte_in_runlength_file()
  {
    return *reinterpret_cast<const uint8_t*>(
        &m_runlengthfile_buffer[m_runlengthfile_index++]);
  }

  uint8_t current_byte_in_runlength_file()
  {
    return *reinterpret_cast<const uint8_t*>(
        &m_runlengthfile_buffer[m_runlengthfile_index]);
  }

  void parse_and_print_log_entry(
      const std::vector<index_file_parser::index_entry>& index_table)
  {
    // First parse the index into the index table

    // If the runlength is > 0, then the index is in the runlength file
    // If the runlength is 0, then the index is in the log file
    // Check runlength first

    std::size_t index;
    std::size_t runlength = 1;

    index = current_byte_in_log_file();

    if (m_runlengthfile_index >= m_runlengthfile_buffer_size) {
      // No more runlengths in the runlength file
      index = next_byte_in_log_file();
    } else {
      const std::size_t index_in_runlength_file =
          current_byte_in_runlength_file();

      if (index_in_runlength_file == index) {
        // The current index has a runlength of some amount
        // parse `runlength` number of args in the log file

        // get past the index in the run length file
        index = next_byte_in_runlength_file();
        next_byte_in_log_file();

        // the runlength is stored as an integer
        // Find out how many bytes the runlength is
        std::size_t integer_width = next_byte_in_runlength_file();

        std::vector<uint8_t> runlength_bytes;
        for (std::size_t i = 0; i < integer_width; ++i) {
          runlength_bytes.push_back(next_byte_in_runlength_file());
        }
        memcpy(&runlength, runlength_bytes.data(), integer_width);
        m_runlengthfile_index += integer_width;
      } else {
        // The current index in the log file
        // has a runlength of one
        index = next_byte_in_log_file();
      }
    }

    if (runlength == 1) {
      index = next_byte_in_log_file();
    }

    auto index_entry = index_table[index];

    // Now we know the format string and the number of arguments
    // along with the type of each argument to be parsed

    for (std::size_t i = 0; i < runlength; ++i) {
      fmt::dynamic_format_arg_store<fmt::format_context> store;
      for (const auto& arg_info : index_entry.args) {
        arg new_arg;
        new_arg.type = arg_info.type;

        if (arg_info.is_constant) {
          // The argument value is in the index table
          new_arg.size = sizeof_arg_type(arg_info.type);
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
            size = next_byte_in_log_file();
          } else {
            // size if the size of the type
            size = sizeof_arg_type(arg_info.type);
          }
          // the next bytes will be the data
          new_arg.size = size;
          new_arg.value = std::string_view(
              reinterpret_cast<const char*>(&m_logfile_buffer[m_logfile_index]),
              size);
          m_logfile_index += size;
        }
        update_store(store, new_arg);
      }
      fmt::print("{}\n", fmt::vformat(index_entry.format_string, store));
    }
  }

  void update_store(fmt::dynamic_format_arg_store<fmt::format_context>& store,
                    arg& arg)
  {
    if (arg.type == binary_log::fmt_arg_type::type_bool) {
      bool value = *(bool*)&arg.value.data()[0];
      store.push_back(value);
    } else if (arg.type == binary_log::fmt_arg_type::type_char) {
      char value = *(char*)&arg.value.data()[0];
      store.push_back(value);
    } else if (arg.type == binary_log::fmt_arg_type::type_uint8) {
      uint8_t value = *(uint8_t*)&arg.value.data()[0];
      store.push_back(value);
      std::cout << "Parsing uint8 " << value << std::endl;
    } else if (arg.type == binary_log::fmt_arg_type::type_uint16) {
      if (arg.size == 1) {
        // actually a uint8_t
        uint8_t value = *(uint8_t*)&arg.value.data()[0];
        store.push_back(value);
      } else {
        uint16_t value = *(uint16_t*)&arg.value.data()[0];
        store.push_back(value);
      }
    } else if (arg.type == binary_log::fmt_arg_type::type_uint32) {
      if (arg.size == 1) {
        // actually a uint8_t
        uint8_t value = *(uint8_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.size == 2) {
        // actually a uint16_t
        uint16_t value = *(uint16_t*)&arg.value.data()[0];
        store.push_back(value);
      } else {
        uint32_t value = *(uint32_t*)&arg.value.data()[0];
        store.push_back(value);
      }
    } else if (arg.type == binary_log::fmt_arg_type::type_uint64) {
      if (arg.size == 1) {
        // actually a uint8_t
        uint8_t value = *(uint8_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.size == 2) {
        // actually a uint16_t
        uint16_t value = *(uint16_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.size == 4) {
        // actually a uint32_t
        uint32_t value = *(uint32_t*)&arg.value.data()[0];
        store.push_back(value);
      } else {
        uint64_t value = *(uint64_t*)&arg.value.data()[0];
        store.push_back(value);
      }
    } else if (arg.type == binary_log::fmt_arg_type::type_int8) {
      int8_t value = *(int8_t*)&arg.value.data()[0];
      store.push_back(value);
    } else if (arg.type == binary_log::fmt_arg_type::type_int16) {
      if (arg.size == 1) {
        // actually a int8_t
        int8_t value = *(int8_t*)&arg.value.data()[0];
        store.push_back(value);
      } else {
        int16_t value = *(int16_t*)&arg.value.data()[0];
        store.push_back(value);
      }
    } else if (arg.type == binary_log::fmt_arg_type::type_int32) {
      if (arg.size == 1) {
        // actually a int8_t
        int8_t value = *(int8_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.size == 2) {
        // actually a int16_t
        int16_t value = *(int16_t*)&arg.value.data()[0];
        store.push_back(value);
      } else {
        int32_t value = *(int32_t*)&arg.value.data()[0];
        store.push_back(value);
      }
    } else if (arg.type == binary_log::fmt_arg_type::type_int64) {
      if (arg.size == 1) {
        // actually a int8_t
        int8_t value = *(int8_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.size == 2) {
        // actually a int16_t
        int16_t value = *(int16_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.size == 4) {
        // actually a int32_t
        int32_t value = *(int32_t*)&arg.value.data()[0];
        store.push_back(value);
      } else {
        int64_t value = *(int64_t*)&arg.value.data()[0];
        store.push_back(value);
      }
    } else if (arg.type == binary_log::fmt_arg_type::type_float) {
      float value = *(float*)&arg.value.data()[0];
      store.push_back(value);
    } else if (arg.type == binary_log::fmt_arg_type::type_double) {
      double value = *(double*)&arg.value.data()[0];
      store.push_back(value);
    } else if (arg.type == binary_log::fmt_arg_type::type_string) {
      std::string value =
          std::string((char*)&arg.value.data()[0], arg.value.size());
      store.push_back(value);
    }
  }

public:
  log_file_parser(const char* log_file_path, const char* runlength_file_path)
  {
    m_logfile_mmap = mio::mmap_source(log_file_path);
    if (!m_logfile_mmap.is_open() || !m_logfile_mmap.is_mapped()) {
      throw std::runtime_error("Could not open log file");
    }

    m_logfile_buffer = m_logfile_mmap.data();
    m_logfile_buffer_size = m_logfile_mmap.mapped_length();

    m_runlengthfile_mmap = mio::mmap_source(runlength_file_path);
    if (!m_runlengthfile_mmap.is_open() || !m_runlengthfile_mmap.is_mapped()) {
      throw std::runtime_error("Could not open runlength file");
    }

    m_runlengthfile_buffer = m_runlengthfile_mmap.data();
    m_runlengthfile_buffer_size = m_runlengthfile_mmap.mapped_length();
  }

  void parse_and_print(
      const std::vector<index_file_parser::index_entry>& index_table)
  {
    while (m_logfile_index < m_logfile_buffer_size) {
      parse_and_print_log_entry(index_table);
    }
  }
};
}  // namespace binary_log
