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
  log_file_parser.parse_and_print(index_entries);
}
