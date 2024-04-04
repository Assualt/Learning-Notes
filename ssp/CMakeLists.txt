cmake_minimum_required(VERSION 3.13.0)
project(ssp_tools VERSION 1.0.0)

option(BUILD_TESTS "Enable GoogleTesting " ON)
option(BUILD_ASAN "Enable Asan memory detector" OFF)
option(BUILD_GUI "Enable build gui window" OFF)

include(cmake/project_init.cmake)
include(cmake/cxx_compile_flags.cmake)
include_directories(${PROJECT_SOURCE_DIR})

if (BUILD_GUI)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
  add_library(ssp-gui-interface INTERFACE)
  target_link_libraries(ssp-gui-interface INTERFACE ${GTKMM_LIBRARIES})
  target_include_directories(ssp-gui-interface INTERFACE ${GTKMM_INCLUDE_DIRS})
  target_link_directories(ssp-gui-interface INTERFACE ${GTKMM_LIBRARY_DIRS})
endif()

add_subdirectory(source)

if(BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory("third_party/googletest")
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  add_subdirectory(test)
endif()

add_library(ssp-internal-interface INTERFACE)
target_include_directories(ssp-internal-interface
    INTERFACE
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/source
)

