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

set(CMAKE_COMMON_FLAGS "-fPIC -fexceptions -fno-common -fno-inline -fno-omit-frame-pointer -fno-strict-aliasing \
-fno-delete-null-pointer-checks -Wunused-but-set-variable -Wunused-variable -fstack-protector-strong -pipe -Werror \
-Wall -Waddress -Wreturn-type")

set(CMAKE_DEBUG_FLAGS "-g")
set(CMAKE_RELASE_FLAGS "-DNDEBUG")

set(CMAKE_CXX_FLAGS "${CMAKE_COMMON_FLAGS} ${CMAKE_LINK_FLAGS} ${CMAKE_DEBUG_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_COMMON_FLAGS} ${CMAKE_LINK_FLAGS} ${CMAKE_DEBUG_FLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-builtin-macro-redefined -U__FILE__ -D_FILENAME_='\"$(notdir $(subst .o,,$(abspath $@)))\"' -D__FILE__='\"$(notdir $(subst .o,,$(abspath $@)))\"'")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-builtin-macro-redefined -U__FILE__ -D_FILENAME_='\"$(notdir $(subst .o,,$(abspath $@)))\"' -D__FILE__='\"$(notdir $(subst .o,,$(abspath $@)))\"'")

add_compile_definitions(
    SUPPORT_OPENSSL=1
)