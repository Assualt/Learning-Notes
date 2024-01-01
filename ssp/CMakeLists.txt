cmake_minimum_required(VERSION 3.13.0)
project(ssp_tools VERSION 1.0.0)

option(BUILD_TESTS "Enable GoogleTesting " ON)
option(BUILD_ASAN "Enable Asan memory detector" OFF)

include(cmake/project_init.cmake)
include(cmake/cxx_compile_flags.cmake)
include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(source)

if(BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory("third_party/googletest")
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  add_subdirectory(test)
endif()

add_library(ssp_base_interface INTERFACE
    source/net/socket.cpp
    source/net/socket.h
    source/net/socket_ops.cpp
    source/net/socket_ops.h
    source/net/socket_typedef.h)
target_sources(ssp_base_interface PUBLIC
FILE_SET HEADERS
BASE_DIRS
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/source
)