cmake_minimum_required(VERSION 3.14)

project(binary_logUnpacker CXX)

include(../../cmake/project-is-top-level.cmake)
include(../../cmake/folders.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(binary_log REQUIRED)
endif()

add_custom_target(run-unpacker)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

function(add_unpacker NAME)
  add_executable("${NAME}" "${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE binary_log::binary_log benchmark::benchmark)
  target_compile_features("${NAME}" PRIVATE cxx_std_20)
  add_custom_target("run_${NAME}" COMMAND "${NAME}" VERBATIM)
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-unpacker "run_${NAME}")
  target_include_directories("${NAME}" PRIVATE .)
endfunction()

add_unpacker(unpacker)

add_folders(Unpacker)
