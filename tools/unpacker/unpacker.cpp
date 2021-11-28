#include <fstream>
#include <iostream>
#include <string>

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

  std::fstream file(index_file_path, std::ios::in | std::ios::binary);

  file.seekp(0, std::ios_base::beg);

  enum class index_file_parser_state
  {
    format_string_length,
    format_string,
    num_args,
    arg_type,
    arg_is_constant,
    arg_value
  };

  auto state = index_file_parser_state::format_string_length;
  std::size_t current_format_string_length {0};
  std::string current_format_string {""};
  std::size_t current_num_args {0};

  while (file) {
    char byte[1];
    file.read(byte, 1);

    switch (state) {
      case index_file_parser_state::format_string_length: {
        current_format_string_length = *reinterpret_cast<uint8_t*>(byte);
        std::cout << "format_string_length: " << current_format_string_length
                  << std::endl;
        state = index_file_parser_state::format_string;
        break;
      }
      case index_file_parser_state::format_string: {
        // std::cout << "format_string: " << byte << std::endl;
        if (current_format_string_length == 0) {
          std::cout << "format_string: " << current_format_string << std::endl;
          state = index_file_parser_state::num_args;
        } else {
          current_format_string += byte[0];
          current_format_string_length--;
        }
        break;
      }
      case index_file_parser_state::num_args: {
        current_num_args = *reinterpret_cast<uint8_t*>(byte);
        std::cout << "num_args: " << current_num_args << std::endl;

        if (current_num_args == 0) {
          // No more parsing needed for this format string / log entry
          state = index_file_parser_state::format_string_length;
        } else {
          state = index_file_parser_state::arg_type;
        }
        break;
      }
      case index_file_parser_state::arg_type: {
        // uint8_t& arg_type = *reinterpret_cast<uint8_t*>(byte);
        // std::cout << "arg_type: " << arg_type << std::endl;
        // state = index_file_parser_state::arg_is_constant;
        break;
      }
      case index_file_parser_state::arg_is_constant: {
        // uint8_t& arg_is_constant = *reinterpret_cast<uint8_t*>(byte);
        // std::cout << "arg_is_constant: " << arg_is_constant << std::endl;
        // state = index_file_parser_state::arg_value;
        break;
      }
      case index_file_parser_state::arg_value: {
        // uint8_t& arg_value = *reinterpret_cast<uint8_t*>(byte);
        // std::cout << "arg_value: " << arg_value << std::endl;
        // state = index_file_parser_state::format_string_length;
        break;
      }
      default:
        break;
    }
  }
}