cmake_minimum_required(VERSION 3.14)

project(binary_logTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)
include(../cmake/windows-set-path.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(binary_log REQUIRED)
  enable_testing()
endif()

add_executable(binary_log_test source/binary_log_test.cpp)
target_link_libraries(binary_log_test PRIVATE binary_log::binary_log)
target_compile_features(binary_log_test PRIVATE cxx_std_20)

add_test(NAME binary_log_test COMMAND binary_log_test)

add_folders(Test)
