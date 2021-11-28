#include <iostream>

#include <argparse.hpp>

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
}