#include <fstream>
#include <iostream>
#include <string>

#include <argparse.hpp>
#include <index_file_parser.hpp>
#include <log_file_parser.hpp>

#define FMT_HEADER_ONLY
#include <fmt/args.h>
#include <fmt/format.h>

int main(int argc, char* argv[])
{
  argparse::ArgumentParser program("unpacker");
  program.add_argument("-i", "--index-file").help("Index file").required();
  program.add_argument("-l", "--log-file").help("Log file").required();

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  auto index_file_path = program.get<std::string>("-i");
  auto log_file_path = program.get<std::string>("-l");

  // Parse index file
  auto index_file_parser =
      binary_log::index_file_parser(index_file_path.c_str());
  auto index_entries = index_file_parser.parse();

  // Parse log file
  auto log_file_parser = binary_log::log_file_parser(log_file_path.c_str());
  auto log_entries = log_file_parser.parse(index_entries);

  // Print log entries
  for (auto& entry : log_entries) {
    const auto& format_string =
        index_entries[entry.format_string_index].format_string;
    fmt::dynamic_format_arg_store<fmt::format_context> store;

    for (auto& arg : entry.args) {
      if (arg.type == binary_log::fmt_arg_type::type_bool) {
        bool value = *(bool*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_char) {
        char value = *(char*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_uint8) {
        uint8_t value = *(uint8_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_uint16) {
        uint16_t value = *(uint16_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_uint32) {
        uint32_t value = *(uint32_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_uint64) {
        uint64_t value = *(uint64_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_int8) {
        int8_t value = *(int8_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_int16) {
        int16_t value = *(int16_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_int32) {
        int32_t value = *(int32_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_int64) {
        int64_t value = *(int64_t*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_float) {
        float value = *(float*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_double) {
        double value = *(double*)&arg.value.data()[0];
        store.push_back(value);
      } else if (arg.type == binary_log::fmt_arg_type::type_string) {
        char* value = (char*)&arg.value.data()[0];
        store.push_back(value);
      }
    }

    fmt::print("{}\n", fmt::vformat(format_string, store));
  }
}