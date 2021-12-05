cmake_minimum_required(VERSION 3.14)

project(binary_logExamples CXX)

include(../../cmake/project-is-top-level.cmake)
include(../../cmake/folders.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(binary_log REQUIRED)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

function(add_example NAME)
  add_executable("${NAME}" "${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE binary_log::binary_log)
  target_compile_features("${NAME}" PRIVATE cxx_std_20)
  add_custom_target("run_${NAME}" COMMAND "${NAME}" VERBATIM)
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-examples "run_${NAME}")  
endfunction()

add_example(billion_integers)

add_folders(Example)
