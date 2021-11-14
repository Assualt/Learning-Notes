cmake_minimum_required(VERSION 2.6)
project(muduo C CXX)

enable_testing()

set (CXX_FLAGS
    -g
    -Wall
    -Werror
    -std=c++11
)
add_definitions("-g -std=c++11")
include_directories(${PROJECT_SOURCE_DIR})
add_subdirectory(base)
add_subdirectory(net)