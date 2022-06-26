cmake_minimum_required(VERSION 2.6)
project(muduo_base_tools)

set(dep_muduo_VERSION_MAJOR 1)
set(dep_muduo_VERSION_MINOR 1)
set(dep_muduo_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

option(BUILD_TESTS "Enable GoogleTesting " ON)
option(BUILD_ASAN "Enable Asan memory detector" OFF)

include(cmake/project_init.cmake)
include(cmake/cxx_compile_flags.cmake)

include_directories(${PROJECT_SOURCE_DIR})
enable_testing()
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
add_subdirectory(base)
add_subdirectory(net)