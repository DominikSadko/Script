cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Wextra -Werror -pedantic")

project(Script VERSION 1.0)

set(SCRIPT_BUILD_TESTS OFF CACHE BOOL "Build script tests")

add_subdirectory(external/)
add_subdirectory(src/)

if (SCRIPT_BUILD_TESTS)
  add_subdirectory(test/)
endif()
