cmake_minimum_required(VERSION 3.20)

project(UnitTest)

file (GLOB SOURCES_TESTS
  src/Framework/Script/*
)

include_directories(UnitTest PRIVATE ${CMAKE_BINARY_DIR}/external/include/)
include_directories(UnitTest PRIVATE ${CMAKE_SOURCE_DIR}/src/)
link_directories(${CMAKE_BINARY_DIR}/lib/)
add_executable(UnitTest ${SOURCES_TESTS})

add_dependencies(UnitTest FrameworkScript)
add_dependencies(UnitTest LuaJIT)
add_dependencies(UnitTest gtest gmock)

target_link_libraries(UnitTest PRIVATE
  FrameworkScript

  gmock
  gtest
  gtest_main
  luajit
)
