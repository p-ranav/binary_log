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
  program.add_argument("log file").help("binary_log log file to deflate");

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  auto log_file_path = program.get<std::string>("log file");
  auto index_file_path = log_file_path + ".index";
  auto runlength_file_path = log_file_path + ".runlength";

  // Parse index file
  auto index_file_parser =
      binary_log::index_file_parser(index_file_path.c_str());
  auto index_entries = index_file_parser.parse();

  // Parse log file
  auto log_file_parser = binary_log::log_file_parser(
      log_file_path.c_str(), runlength_file_path.c_str());
  log_file_parser.parse_and_print(index_entries);
}
