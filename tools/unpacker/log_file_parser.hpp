#pragma once
#include <string_view>
#include <vector>

#include <binary_log/packer.hpp>
#include <mio.hpp>

namespace binary_log
{
class log_file_parser
{
  mio::mmap_source m_mmap;
  const char* m_buffer {nullptr};
  size_t m_buffer_size {0};

  std::size_t m_index {0};  // into m_buffer

  struct arg {
	std::vector<uint8_t> arg_data;
  };

  struct log_entry
  {
    std::size_t format_string_index;
    std::vector<arg> args;
  };  

  void parse_entry()
  {
    log_entry entry;

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

  void parse(/* Take index table as argument? */)
  {
    while (m_index < m_buffer_size) {
      parse_entry();
    }
  }
};
}