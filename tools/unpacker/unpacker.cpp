#include <fstream>
#include <iostream>
#include <string>

#include <argparse.hpp>
#include <index_file_parser.hpp>
#include <log_file_parser.hpp>

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

  std::cout << index_file_path << std::endl;
  std::cout << log_file_path << std::endl;

  auto index_file_parser =
      binary_log::index_file_parser(index_file_path.c_str());
  auto entries = index_file_parser.parse();

  for (auto& entry : entries) {
    std::cout << entry.format_string << " " << entry.args.size() << " args\n";
    for (auto& arg : entry.args) {
      std::cout << "Arg:\n";
      std::cout << "  type                "
                << binary_log::packer::datatype_to_string(arg.type) << "\n";
      std::cout << "  is constant?        " << std::boolalpha << arg.is_constant
                << "\n";
      std::cout << "  data in index file? " << std::boolalpha
                << (arg.arg_data.size() > 0) << "\n";
      std::cout << "---\n";
    }
  }
}