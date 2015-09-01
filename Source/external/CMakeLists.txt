cmake_minimum_required(VERSION 3.20)

include(ExternalProject)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/external/include/)

if (SCRIPT_BUILD_TESTS)
  add_subdirectory(googletest)
  target_compile_options(gtest PRIVATE "-Wno-implicit-int-float-conversion")
  target_include_directories(gtest PRIVATE include)
endif()

ExternalProject_Add(LuaJIT
  SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/lua/
  PREFIX            ${CMAKE_CURRENT_BINARY_DIR}

  UPDATE_COMMAND    ""
  CONFIGURE_COMMAND ""
  PATCH_COMMAND     patch -Np1 -r - -i ${CMAKE_CURRENT_SOURCE_DIR}/luajit210.patch || true

  BUILD_COMMAND
    COMMAND ${CMAKE_COMMAND} -E env
    HOST_CC=clang
    CC=${CMAKE_C_COMPILER}
    make -C <SOURCE_DIR>
  BUILD_IN_SOURCE   TRUE

  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy
      <SOURCE_DIR>/src/lauxlib.h
      <SOURCE_DIR>/src/lua.h
      <SOURCE_DIR>/src/lua.hpp
      <SOURCE_DIR>/src/luaconf.h
      <SOURCE_DIR>/src/luajit.h
      <SOURCE_DIR>/src/lualib.h
      ${CMAKE_BINARY_DIR}/external/include/
    COMMAND ${CMAKE_COMMAND} -E copy
      <SOURCE_DIR>/src/libluajit.a
      ${CMAKE_BINARY_DIR}/lib/
)
