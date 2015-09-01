cmake_minimum_required(VERSION 3.20)

file (GLOB SOURCES_FRAMEWORK_SCRIPT
  Framework/Script/*.cpp
  Framework/Script/Stack/*.cpp
)
file (GLOB HEADERS_FRAMEWORK_SCRIPT
  Framework/Script/*.hpp
  Framework/Script/Stack/*.hpp
)

add_library(FrameworkScript SHARED ${SOURCES_FRAMEWORK_SCRIPT} ${HEADERS_FRAMEWORK_SCRIPT})

add_dependencies(FrameworkScript LuaJIT)
target_include_directories(FrameworkScript PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(FrameworkScript PUBLIC ${CMAKE_BINARY_DIR}/external/include/)
set_target_properties(FrameworkScript PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set_target_properties(FrameworkScript PROPERTIES OUTPUT_NAME "framework_script")
