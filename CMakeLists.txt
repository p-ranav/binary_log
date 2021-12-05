cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    binary_log
    VERSION 0.1.0
    DESCRIPTION "Fast binary logger C++20"
    HOMEPAGE_URL "https://github.com/p-ranav/binary_log"
    LANGUAGES C CXX
)

docs_early_return()

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(binary_log_binary_log INTERFACE)
add_library(binary_log::binary_log ALIAS binary_log_binary_log)

set_property(
    TARGET binary_log_binary_log PROPERTY
    EXPORT_NAME binary_log
)

target_include_directories(
    binary_log_binary_log ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_link_libraries(binary_log_binary_log INTERFACE)

target_compile_features(binary_log_binary_log INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Unpacker ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_UNPACKER "Build unpacker tree." "${binary_log_DEVELOPER_MODE}")
  if(BUILD_UNPACKER)
    add_subdirectory(tools)
  endif()
endif()

# ---- Benchmarks ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_BENCHMARKS "Build benchmarks tree." "${binary_log_DEVELOPER_MODE}")
  if(BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
  endif()
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${binary_log_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(examples)
  endif()
endif()

# ---- Developer mode ----

if(NOT binary_log_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of binary_log"
  )
endif()

include(cmake/dev-mode.cmake)
