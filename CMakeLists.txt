cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    binary_log
    VERSION 0.1.0
    DESCRIPTION "Fast asynchronous binary logger for modern C++"
    HOMEPAGE_URL "https://github.com/p-ranav/binary_log"
    LANGUAGES C CXX
)

docs_early_return()

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Find dependencies ----
find_package(Threads REQUIRED)

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
    ${fmt_INCLUDE_DIRS}
    ${concurrentqueue_INCLUDE_DIRS}
)

target_link_libraries(binary_log_binary_log INTERFACE Threads::Threads)

target_compile_features(binary_log_binary_log INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Examples ----

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLES "Build examples tree." "${binary_log_DEVELOPER_MODE}")
  if(BUILD_EXAMPLES)
    add_subdirectory(example)
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
