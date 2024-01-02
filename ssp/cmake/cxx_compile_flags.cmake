set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_definitions("-g")
set(BUILD_ASAN OFF)

if (BUILD_ASAN)
  add_definitions("-fsanitize=address -fno-omit-frame-pointer -lasan")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -lasan")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -lasan")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fno-omit-frame-pointer -lasan")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address -fno-omit-frame-pointer -lasan")
  message(STATUS "optional: asan option is on")
endif(BUILD_ASAN)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  add_compile_definitions(-DLINUX)
else ()
  message(STATUS "define apple")
endif ()
message(STATUS "cmake _system_name ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM}")

add_compile_definitions(
    SUPPORT_OPENSSL=1
)