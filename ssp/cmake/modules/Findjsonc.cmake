include(ExternalProject)
message(STATUS "begin to find json-c")

set(jsonc_ROOT         ${PROJECT_SOURCE_DIR}/third_party/json-c)
# 指定配置指令（注意此处修改了安装目录，否则默认情况下回安装到系统目录）
set(jsonc_CONFIGURE    cd ${jsonc_ROOT}/ && rm -rf build && mkdir -p build && cd build && cmake -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/ -DBUILD_TESTING=OFF -DBUILD_APPS=OFF ..)
# 指定编译指令（需要覆盖默认指令，进入我们指定的jsonc_ROOT目录下）
set(jsonc_MAKE         cd ${jsonc_ROOT}/build && make -j4)
# 指定安装指令（需要覆盖默认指令，进入我们指定的jsonc_ROOT目录下）
set(jsonc_INSTALL      cd ${jsonc_ROOT}/build && make install)

find_path(Jsonc_INCLUDE_DIRS NAMES json-c/json.h PATHS "${CMAKE_BINARY_DIR}/install/include/")
find_library(Jsonc_LIBRARIES NAMES libjson-c.a PATHS "${CMAKE_BINARY_DIR}/install/lib")

if (Jsonc_INCLUDE_DIRS AND Jsonc_LIBRARIES)
    set(jsonc_FOUND TRUE)
    message(STATUS "jsonc found!!!!")
    return()
endif()

message(STATUS "jsonc not found! and begin to configure it")

# CMakeLists.txt
# 输出操作系统名称
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove_directory ${jsonc_ROOT}/build)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${jsonc_ROOT}/build)
EXECUTE_PROCESS(COMMAND cmake -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/ -DBUILD_TESTING=OFF -DBUILD_APPS=OFF ..
    WORKING_DIRECTORY ${jsonc_ROOT}/build
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE result
)

if (result)
    message(FATAL_ERROR "jsonc configure failed")
endif()

EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} --build . --target install
    WORKING_DIRECTORY ${jsonc_ROOT}/build
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE result)

if (result)
    message(FATAL_ERROR "jsonc build failed")
endif()

find_path(Jsonc_INCLUDE_DIRS NAMES json-c/json.h PATHS "${CMAKE_BINARY_DIR}/install/include/")
find_library(Jsonc_LIBRARIES NAMES libjson-c.a PATHS "${CMAKE_BINARY_DIR}/install/lib")
set(jsonc_FOUND TRUE)
