set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(BUILD_ASAN OFF)

if (BUILD_ASAN)
  add_definitions("-fsanitize=address -fno-omit-frame-pointer -lasan")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -lasan")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -lasan")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fno-omit-frame-pointer -lasan")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address -fno-omit-frame-pointer -lasan")
  message(STATUS "optional: asan option is on")
endif(BUILD_ASAN)

set(PLATFORM "")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_definitions(-DLINUX)
elseif (MSVC)
  add_definitions(-DWIN32)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DMAC_OS)
endif ()
message(STATUS "cmake _system_name ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM}")

set(COMPILE_DEBUG_FLAGS "-g")
set(COMPILE_FLAGS "")
string(APPEND COMPILE_FLAGS "-fsigned-char ${COMPILE_DEBUG_FLAGS} -Wall")
string(APPEND CMAKE_CXX_FLAGS ${COMPILE_FLAGS})
string(APPEND CMAKE_C_FLAGS ${COMPILE_FLAGS})

add_compile_definitions(
    SUPPORT_OPENSSL=1
)