#include <filesystem>
#include <fstream>
#include <vector>

#include <binary_log/binary_log.hpp>
#include <doctest.hpp>

using doctest::test_suite;

static constexpr auto test_file = "test.log";
static constexpr auto test_file_index = "test.log.index";
static constexpr auto test_file_runlength = "test.log.runlength";

auto read_file(std::string_view path) -> std::vector<uint8_t>
{
  std::ifstream infile(path.data(), std::ios::binary);
  return std::vector<uint8_t>(std::istreambuf_iterator<char>(infile), {});
}

TEST_CASE("binary_log can pack format strings" * test_suite("packer"))
{
  {
    binary_log::binary_log log(test_file);
    BINARY_LOG(log, "Hello, world!");
  }
  // The file should be flushed and closed.
  REQUIRE(std::filesystem::exists(test_file));
  REQUIRE(std::filesystem::exists(test_file_index));
  REQUIRE(std::filesystem::exists(test_file_runlength));

  auto log_file = read_file(test_file);
  auto index_file = read_file(test_file_index);
  auto runlength_file = read_file(test_file_runlength);

  // Runlength file should be empty.
  REQUIRE(runlength_file.empty());

  REQUIRE(index_file.size() == 15);
  REQUIRE(index_file[0] == 0x0d);  // format string length
  REQUIRE(index_file[1] == 0x48);  // 'H'
  REQUIRE(index_file[2] == 0x65);  // 'e'
  REQUIRE(index_file[3] == 0x6c);  // 'l'
  REQUIRE(index_file[4] == 0x6c);  // 'l'
  REQUIRE(index_file[5] == 0x6f);  // 'o'
  REQUIRE(index_file[6] == 0x2c);  // ','
  REQUIRE(index_file[7] == 0x20);  // ' '
  REQUIRE(index_file[8] == 0x77);  // 'w'
  REQUIRE(index_file[9] == 0x6f);  // 'o'
  REQUIRE(index_file[10] == 0x72);  // 'r'
  REQUIRE(index_file[11] == 0x6c);  // 'l'
  REQUIRE(index_file[12] == 0x64);  // 'd'
  REQUIRE(index_file[13] == 0x21);  // '!'
  REQUIRE(index_file[14] == 0x00);  // num_args == 0

  REQUIRE(log_file.size() == 1);
  REQUIRE(log_file[0] == 0x00);  // index of 0, referring to row in index file

  remove(test_file);
  remove(test_file_index);
  remove(test_file_runlength);
}

TEST_CASE("binary_log can pack string arguments" * test_suite("packer"))
{
  {
    binary_log::binary_log log(test_file);
    BINARY_LOG(log, "Hello, {}!", "world");
  }
  // The file should be flushed and closed.
  REQUIRE(std::filesystem::exists(test_file));
  REQUIRE(std::filesystem::exists(test_file_index));
  REQUIRE(std::filesystem::exists(test_file_runlength));

  auto log_file = read_file(test_file);
  auto index_file = read_file(test_file_index);
  auto runlength_file = read_file(test_file_runlength);

  // Runlength file should be empty.
  REQUIRE(runlength_file.empty());

  REQUIRE(index_file.size() == 14);
  REQUIRE(index_file[0] == 0x0a);  // format string length
  REQUIRE(index_file[1] == 0x48);  // 'H'
  REQUIRE(index_file[2] == 0x65);  // 'e'
  REQUIRE(index_file[3] == 0x6c);  // 'l'
  REQUIRE(index_file[4] == 0x6c);  // 'l'
  REQUIRE(index_file[5] == 0x6f);  // 'o'
  REQUIRE(index_file[6] == 0x2c);  // ','
  REQUIRE(index_file[7] == 0x20);  // ' '
  REQUIRE(index_file[8] == 0x7b);  // '{'
  REQUIRE(index_file[9] == 0x7d);  // '}'
  REQUIRE(index_file[10] == 0x21);  // '!'
  REQUIRE(index_file[11] == 0x01);  // num_args == 1
  REQUIRE(index_file[12]
          == static_cast<uint8_t>(
              binary_log::fmt_arg_type::type_string));  // arg type is string
  REQUIRE(index_file[13] == 0x00);  // not constant

  REQUIRE(log_file.size() == 7);
  REQUIRE(log_file[0] == 0x00);  // index of 0, referring to row in index file
  REQUIRE(log_file[1] == 0x05);  // length of "world"
  REQUIRE(log_file[2] == 0x77);  // 'w'
  REQUIRE(log_file[3] == 0x6f);  // 'o'
  REQUIRE(log_file[4] == 0x72);  // 'r'
  REQUIRE(log_file[5] == 0x6c);  // 'l'
  REQUIRE(log_file[6] == 0x64);  // 'd'

  remove(test_file);
  remove(test_file_index);
  remove(test_file_runlength);
}

TEST_CASE("binary_log can pack int32 arguments" * test_suite("packer"))
{
  {
    binary_log::binary_log log(test_file);
    BINARY_LOG(log, "Hello, {}!", 42);
  }
  // The file should be flushed and closed.
  REQUIRE(std::filesystem::exists(test_file));
  REQUIRE(std::filesystem::exists(test_file_index));
  REQUIRE(std::filesystem::exists(test_file_runlength));

  auto log_file = read_file(test_file);
  auto index_file = read_file(test_file_index);
  auto runlength_file = read_file(test_file_runlength);

  // Runlength file should be empty.
  REQUIRE(runlength_file.empty());

  REQUIRE(index_file.size() == 14);
  REQUIRE(index_file[0] == 0x0a);  // format string length
  REQUIRE(index_file[1] == 0x48);  // 'H'
  REQUIRE(index_file[2] == 0x65);  // 'e'
  REQUIRE(index_file[3] == 0x6c);  // 'l'
  REQUIRE(index_file[4] == 0x6c);  // 'l'
  REQUIRE(index_file[5] == 0x6f);  // 'o'
  REQUIRE(index_file[6] == 0x2c);  // ','
  REQUIRE(index_file[7] == 0x20);  // ' '
  REQUIRE(index_file[8] == 0x7b);  // '{'
  REQUIRE(index_file[9] == 0x7d);  // '}'
  REQUIRE(index_file[10] == 0x21);  // '!'
  REQUIRE(index_file[11] == 0x01);  // num_args == 1
  REQUIRE(index_file[12]
          == static_cast<uint8_t>(
              binary_log::fmt_arg_type::type_int32));  // arg type is int
  REQUIRE(index_file[13] == 0x00);  // not constant

  REQUIRE(log_file.size() == 3);
  REQUIRE(log_file[0] == 0x00);  // index of 0, referring to row in index file
  REQUIRE(log_file[1] == 0x01);  // the integer is 1 byte long
  REQUIRE(log_file[2] == 0x2a);  // 42

  remove(test_file);
  remove(test_file_index);
  remove(test_file_runlength);
}

TEST_CASE("binary_log can pack int64 arguments" * test_suite("packer"))
{
  {
    binary_log::binary_log log(test_file);
    BINARY_LOG(log, "Hello, {}!", 5432123456789);
  }
  // The file should be flushed and closed.
  REQUIRE(std::filesystem::exists(test_file));
  REQUIRE(std::filesystem::exists(test_file_index));
  REQUIRE(std::filesystem::exists(test_file_runlength));

  auto log_file = read_file(test_file);
  auto index_file = read_file(test_file_index);
  auto runlength_file = read_file(test_file_runlength);

  // Runlength file should be empty.
  REQUIRE(runlength_file.empty());

  REQUIRE(index_file.size() == 14);
  REQUIRE(index_file[0] == 0x0a);  // format string length
  REQUIRE(index_file[1] == 0x48);  // 'H'
  REQUIRE(index_file[2] == 0x65);  // 'e'
  REQUIRE(index_file[3] == 0x6c);  // 'l'
  REQUIRE(index_file[4] == 0x6c);  // 'l'
  REQUIRE(index_file[5] == 0x6f);  // 'o'
  REQUIRE(index_file[6] == 0x2c);  // ','
  REQUIRE(index_file[7] == 0x20);  // ' '
  REQUIRE(index_file[8] == 0x7b);  // '{'
  REQUIRE(index_file[9] == 0x7d);  // '}'
  REQUIRE(index_file[10] == 0x21);  // '!'
  REQUIRE(index_file[11] == 0x01);  // num_args == 1
  REQUIRE(index_file[12]
          == static_cast<uint8_t>(binary_log::fmt_arg_type::type_int64));
  REQUIRE(index_file[13] == 0x00);  // not constant

  REQUIRE(log_file.size() == 10);
  REQUIRE(log_file[0] == 0x00);  // index of 0, referring to row in index file
  REQUIRE(log_file[1] == 0x08);  // the integer is 8 bytes long
  REQUIRE(log_file[2] == 0x15);  // 5432123456789
  REQUIRE(log_file[3] == 0xfd);
  REQUIRE(log_file[4] == 0xc9);
  REQUIRE(log_file[5] == 0xc3);
  REQUIRE(log_file[6] == 0xf0);
  REQUIRE(log_file[7] == 0x04);
  REQUIRE(log_file[8] == 0x00);
  REQUIRE(log_file[9] == 0x00);

  remove(test_file);
  remove(test_file_index);
  remove(test_file_runlength);
}

TEST_CASE("binary_log can pack double arguments" * test_suite("packer"))
{
  {
    binary_log::binary_log log(test_file);
    BINARY_LOG(log, "Hello, {}!", 3.14159);
  }
  // The file should be flushed and closed.
  REQUIRE(std::filesystem::exists(test_file));
  REQUIRE(std::filesystem::exists(test_file_index));
  REQUIRE(std::filesystem::exists(test_file_runlength));

  auto log_file = read_file(test_file);
  auto index_file = read_file(test_file_index);
  auto runlength_file = read_file(test_file_runlength);

  // Runlength file should be empty.
  REQUIRE(runlength_file.empty());

  REQUIRE(index_file.size() == 14);
  REQUIRE(index_file[0] == 0x0a);  // format string length
  REQUIRE(index_file[1] == 0x48);  // 'H'
  REQUIRE(index_file[2] == 0x65);  // 'e'
  REQUIRE(index_file[3] == 0x6c);  // 'l'
  REQUIRE(index_file[4] == 0x6c);  // 'l'
  REQUIRE(index_file[5] == 0x6f);  // 'o'
  REQUIRE(index_file[6] == 0x2c);  // ','
  REQUIRE(index_file[7] == 0x20);  // ' '
  REQUIRE(index_file[8] == 0x7b);  // '{'
  REQUIRE(index_file[9] == 0x7d);  // '}'
  REQUIRE(index_file[10] == 0x21);  // '!'
  REQUIRE(index_file[11] == 0x01);  // num_args == 1
  REQUIRE(index_file[12]
          == static_cast<uint8_t>(
              binary_log::fmt_arg_type::type_double));  // arg type is double
  REQUIRE(index_file[13] == 0x00);  // not constant

  REQUIRE(log_file.size() == 9);
  REQUIRE(log_file[0] == 0x00);  // index of 0, referring to row in index file
  // The next bytes are 3.14159 as a double.
  REQUIRE(log_file[1] == 0x6e);
  REQUIRE(log_file[2] == 0x86);
  REQUIRE(log_file[3] == 0x1b);
  REQUIRE(log_file[4] == 0xf0);
  REQUIRE(log_file[5] == 0xf9);
  REQUIRE(log_file[6] == 0x21);
  REQUIRE(log_file[7] == 0x09);
  REQUIRE(log_file[8] == 0x40);

  remove(test_file);
  remove(test_file_index);
  remove(test_file_runlength);
}

TEST_CASE("binary_log can pack constants" * test_suite("packer"))
{
  {
    binary_log::binary_log log(test_file);
    BINARY_LOG(log,
               "Str: {}, Int: {}, Float: {}, Bool: {}",
               binary_log::constant("Hello"),
               binary_log::constant(42),
               binary_log::constant(3.14159f),
               binary_log::constant(true));
  }
  // The file should be flushed and closed.
  REQUIRE(std::filesystem::exists(test_file));
  REQUIRE(std::filesystem::exists(test_file_index));
  REQUIRE(std::filesystem::exists(test_file_runlength));

  auto log_file = read_file(test_file);
  auto index_file = read_file(test_file_index);
  auto runlength_file = read_file(test_file_runlength);

  // Runlength file should be empty.
  REQUIRE(runlength_file.empty());

  REQUIRE(index_file.size() == 62);

  REQUIRE(index_file[0] == 0x25);  // format string length
  REQUIRE(index_file[1] == 0x53);  // 'S'
  REQUIRE(index_file[2] == 0x74);  // 't'
  REQUIRE(index_file[3] == 0x72);  // 'r'
  REQUIRE(index_file[4] == 0x3a);  // ':'
  REQUIRE(index_file[5] == 0x20);  // ' '
  REQUIRE(index_file[6] == 0x7b);  // '{'
  REQUIRE(index_file[7] == 0x7d);  // '}'
  REQUIRE(index_file[8] == 0x2c);  // ','
  REQUIRE(index_file[9] == 0x20);  // ' '
  REQUIRE(index_file[10] == 0x49);  // 'I'
  REQUIRE(index_file[11] == 0x6e);  // 'n'
  REQUIRE(index_file[12] == 0x74);  // 't'
  REQUIRE(index_file[13] == 0x3a);  // ':'
  REQUIRE(index_file[14] == 0x20);  // ' '
  REQUIRE(index_file[15] == 0x7b);  // '{'
  REQUIRE(index_file[16] == 0x7d);  // '}'
  REQUIRE(index_file[17] == 0x2c);  // ','
  REQUIRE(index_file[18] == 0x20);  // ' '
  REQUIRE(index_file[19] == 0x46);  // 'F'
  REQUIRE(index_file[20] == 0x6c);  // 'l'
  REQUIRE(index_file[21] == 0x6f);  // 'o'
  REQUIRE(index_file[22] == 0x61);  // 'a'
  REQUIRE(index_file[23] == 0x74);  // 't'
  REQUIRE(index_file[24] == 0x3a);  // ':'
  REQUIRE(index_file[25] == 0x20);  // ' '
  REQUIRE(index_file[26] == 0x7b);  // '{'
  REQUIRE(index_file[27] == 0x7d);  // '}'
  REQUIRE(index_file[28] == 0x2c);  // ','
  REQUIRE(index_file[29] == 0x20);  // ' '
  REQUIRE(index_file[30] == 0x42);  // 'B'
  REQUIRE(index_file[31] == 0x6f);  // 'o'
  REQUIRE(index_file[32] == 0x6f);  // 'o'
  REQUIRE(index_file[33] == 0x6c);  // 'l'
  REQUIRE(index_file[34] == 0x3a);  // ':'
  REQUIRE(index_file[35] == 0x20);  // ' '
  REQUIRE(index_file[36] == 0x7b);  // '{'
  REQUIRE(index_file[37] == 0x7d);  // '}'
  REQUIRE(index_file[38] == 0x04);  // num_args == 4
  REQUIRE(index_file[39]
          == static_cast<uint8_t>(
              binary_log::fmt_arg_type::type_string));  // arg type is string
  REQUIRE(index_file[40]
          == static_cast<uint8_t>(
              binary_log::fmt_arg_type::type_int32));  // arg type is int
  REQUIRE(index_file[41]
          == static_cast<uint8_t>(
              binary_log::fmt_arg_type::type_float));  // arg type is float
  REQUIRE(index_file[42]
          == static_cast<uint8_t>(
              binary_log::fmt_arg_type::type_bool));  // arg type is bool
  REQUIRE(index_file[43] == 0x01);  // arg1 is a constant
  REQUIRE(index_file[44] == 0x05);  // arg1 string length
  REQUIRE(index_file[45] == 0x48);  // 'H'
  REQUIRE(index_file[46] == 0x65);  // 'e'
  REQUIRE(index_file[47] == 0x6c);  // 'l'
  REQUIRE(index_file[48] == 0x6c);  // 'l'
  REQUIRE(index_file[49] == 0x6f);  // 'o'
  REQUIRE(index_file[50] == 0x01);  // arg2 is a constant
  REQUIRE(index_file[51] == 0x2a);  // arg2 is 42
  REQUIRE(index_file[52] == 0x00);
  REQUIRE(index_file[53] == 0x00);
  REQUIRE(index_file[54] == 0x00);
  REQUIRE(index_file[55] == 0x01);  // arg3 is a constant
  REQUIRE(index_file[56] == 0xd0);  // arg3 is 3.14159f
  REQUIRE(index_file[57] == 0x0f);
  REQUIRE(index_file[58] == 0x49);
  REQUIRE(index_file[59] == 0x40);
  REQUIRE(index_file[60] == 0x01);  // arg4 is a constant
  REQUIRE(index_file[61] == 0x01);  // arg4 is true

  REQUIRE(log_file.size() == 1);
  REQUIRE(log_file[0] == 0x00);  // index of 0, referring to row in index file

  remove(test_file);
  remove(test_file_index);
  remove(test_file_runlength);
}

TEST_CASE("binary_log can run-length encode repeated log calls without args"
          * test_suite("packer"))
{
  {
    binary_log::binary_log log(test_file);
    for (std::size_t i = 0; i < 3; ++i) {
      BINARY_LOG(log, "Thread started");
    }
  }
  // The file should be flushed and closed.
  REQUIRE(std::filesystem::exists(test_file));
  REQUIRE(std::filesystem::exists(test_file_index));
  REQUIRE(std::filesystem::exists(test_file_runlength));

  auto log_file = read_file(test_file);
  auto index_file = read_file(test_file_index);
  auto runlength_file = read_file(test_file_runlength);

  REQUIRE(std::filesystem::file_size(test_file_runlength) == 3);
  // 1 byte for the index
  // 2 bytes for the runlength
  REQUIRE(runlength_file[0]
          == 0x00);  // index of 0, referring to row in index file
  REQUIRE(runlength_file[1] == 0x01);  // it's a uint8_t
  REQUIRE(runlength_file[2] == 0x03);  // the runlength is 3

  REQUIRE(log_file.size() == 1);
  REQUIRE(log_file[0] == 0x00);  // index of 0, referring to row in index file

  REQUIRE(index_file.size() == 16);
  REQUIRE(index_file[0] == 0x0e);  // Length of the format string
  REQUIRE(index_file[1] == 0x54);  // 'T'
  REQUIRE(index_file[2] == 0x68);  // 'h'
  REQUIRE(index_file[3] == 0x72);  // 'r'
  REQUIRE(index_file[4] == 0x65);  // 'e'
  REQUIRE(index_file[5] == 0x61);  // 'a'
  REQUIRE(index_file[6] == 0x64);  // 'd'
  REQUIRE(index_file[7] == 0x20);  // ' '
  REQUIRE(index_file[8] == 0x73);  // 's'
  REQUIRE(index_file[9] == 0x74);  // 't'
  REQUIRE(index_file[10] == 0x61);  // 'a'
  REQUIRE(index_file[11] == 0x72);  // 'r'
  REQUIRE(index_file[12] == 0x74);  // 't'
  REQUIRE(index_file[13] == 0x65);  // 'e'
  REQUIRE(index_file[14] == 0x64);  // 'd'
  REQUIRE(index_file[15] == 0x00);  // num_args is 0

  remove(test_file);
  remove(test_file_index);
  remove(test_file_runlength);
}

TEST_CASE("binary_log can run-length encode repeated log calls with args"
          * test_suite("packer"))
{
  {
    binary_log::binary_log log(test_file);
    for (std::size_t i = 0; i < 3; ++i) {
      BINARY_LOG(log, "Thread {} started", i);
    }
  }
  // The file should be flushed and closed.
  REQUIRE(std::filesystem::exists(test_file));
  REQUIRE(std::filesystem::exists(test_file_index));
  REQUIRE(std::filesystem::exists(test_file_runlength));

  auto log_file = read_file(test_file);
  auto index_file = read_file(test_file_index);
  auto runlength_file = read_file(test_file_runlength);

  REQUIRE(std::filesystem::file_size(test_file_runlength) == 3);
  // 1 byte for the index
  // 2 bytes for the runlength
  REQUIRE(runlength_file[0]
          == 0x00);  // index of 0, referring to row in index file
  REQUIRE(runlength_file[1] == 0x01);  // it's a uint8_t
  REQUIRE(runlength_file[2] == 0x03);  // the runlength is 3

  REQUIRE(log_file.size() == 7);
  REQUIRE(log_file[0] == 0x00);  // index of 0, referring to row in index file
  REQUIRE(log_file[1] == 0x01);  // arg_1 width is 1
  REQUIRE(log_file[2] == 0x00);  // arg_1 value
  REQUIRE(log_file[3] == 0x01);  // arg_2 width is 1
  REQUIRE(log_file[4] == 0x01);  // arg_2 value
  REQUIRE(log_file[5] == 0x01);  // arg_3 width is 1
  REQUIRE(log_file[6] == 0x02);  // arg_3 value

  REQUIRE(index_file.size() == 21);
  REQUIRE(index_file[0] == 0x11);  // Length of the format string
  REQUIRE(index_file[1] == 0x54);  // 'T'
  REQUIRE(index_file[2] == 0x68);  // 'h'
  REQUIRE(index_file[3] == 0x72);  // 'r'
  REQUIRE(index_file[4] == 0x65);  // 'e'
  REQUIRE(index_file[5] == 0x61);  // 'a'
  REQUIRE(index_file[6] == 0x64);  // 'd'
  REQUIRE(index_file[7] == 0x20);  // ' '
  REQUIRE(index_file[8] == 0x7b);  // '{'
  REQUIRE(index_file[9] == 0x7d);  // '}'
  REQUIRE(index_file[10] == 0x20);  // ' '
  REQUIRE(index_file[11] == 0x73);  // 's'
  REQUIRE(index_file[12] == 0x74);  // 't'
  REQUIRE(index_file[13] == 0x61);  // 'a'
  REQUIRE(index_file[14] == 0x72);  // 'r'
  REQUIRE(index_file[15] == 0x74);  // 't'
  REQUIRE(index_file[16] == 0x65);  // 'e'
  REQUIRE(index_file[17] == 0x64);  // 'd'
  REQUIRE(index_file[18] == 0x01);  // num_args is 1
  REQUIRE(index_file[19] == 0x05);  // arg_1 is an uint64_t
  REQUIRE(index_file[20] == 0x00);  // arg_1 is not a constant

  remove(test_file);
  remove(test_file_index);
  remove(test_file_runlength);
}