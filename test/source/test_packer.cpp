#include <filesystem>
#include <fstream>

#include <binary_log/binary_log.hpp>
#include <doctest.hpp>

using doctest::test_suite;

static constexpr auto test_file = "test.log";
static constexpr auto test_file_index = "test.log.index";
static constexpr auto test_file_runlength = "test.log.runlength";

auto read_file(std::string_view path) -> std::string
{
  constexpr auto read_size = std::size_t {4096};
  auto stream = std::ifstream {path.data()};
  stream.exceptions(std::ios_base::badbit);

  auto out = std::string {};
  auto buf = std::string(read_size, '\0');
  while (stream.read(&buf[0], read_size)) {
    out.append(buf, 0, stream.gcount());
  }
  out.append(buf, 0, stream.gcount());
  return out;
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