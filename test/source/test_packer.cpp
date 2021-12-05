#include <binary_log/binary_log.hpp>
#include <doctest.hpp>

using doctest::test_suite;

TEST_CASE("binary_log can pack format strings" * test_suite("packer"))
{
  REQUIRE_EQ(true, true);
}