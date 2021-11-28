cmake_minimum_required(VERSION 3.14)

project(binary_logBenchmark CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(binary_log REQUIRED)
endif()

find_package(benchmark CONFIG)
if(NOT benchmark_FOUND)
    message(STATUS "Using CMake Version ${CMAKE_VERSION}")
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.11.0")
        # User can fetch googlebenchmark
        message(STATUS "Downloading GoogleBenchmark")
        include(FetchContent)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE INTERNAL "")
        # Do not build and run googlebenchmark tests
        FetchContent_Declare(googlebenchmark GIT_REPOSITORY https://github.com/google/benchmark.git GIT_TAG v1.5.2)

        FetchContent_MakeAvailable(googlebenchmark)
    else()
        message(FATAL_ERROR "GoogleBenchmark is missing. Use CMake >= 3.11 or download it")
    endif()
endif()

add_custom_target(run-benchmarks)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

function(add_benchmark NAME)
  add_executable("${NAME}" "${NAME}.cpp")
  target_link_libraries("${NAME}" PRIVATE binary_log::binary_log benchmark::benchmark)
  target_compile_features("${NAME}" PRIVATE cxx_std_20)
  add_custom_target("run_${NAME}" COMMAND "${NAME}" VERBATIM)
  add_dependencies("run_${NAME}" "${NAME}")
  add_dependencies(run-benchmarks "run_${NAME}")  
endfunction()

add_benchmark(binary_log_benchmark)

add_folders(Benchmark)
