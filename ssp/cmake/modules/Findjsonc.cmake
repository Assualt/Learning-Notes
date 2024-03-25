include(ExternalProject)
message(STATUS "begin to find json-c")
set(jsonc_FOUND TRUE)

set(jsonc_ROOT         ${PROJECT_SOURCE_DIR}/third_party/json-c)
# 指定配置指令（注意此处修改了安装目录，否则默认情况下回安装到系统目录）
set(jsonc_CONFIGURE    cd ${jsonc_ROOT}/ && rm -rf build && mkdir -p build && cd build && cmake -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install/ -DBUILD_TESTING=OFF -DBUILD_APPS=OFF ..)
# 指定编译指令（需要覆盖默认指令，进入我们指定的jsonc_ROOT目录下）
set(jsonc_MAKE         cd ${jsonc_ROOT}/build && make -j4)
# 指定安装指令（需要覆盖默认指令，进入我们指定的jsonc_ROOT目录下）
set(jsonc_INSTALL      cd ${jsonc_ROOT}/build && make install)

ExternalProject_Add(Jsonc
    PREFIX            ${jsonc_ROOT}
    SOURCE_DIR        ${jsonc_ROOT}
    CONFIGURE_COMMAND ${jsonc_CONFIGURE}
    BUILD_COMMAND     ${jsonc_MAKE} && ${jsonc_INSTALL}
    INSTALL_COMMAND   ${jsonc_INSTALL}
    BUILD_ALWAYS      1
)

find_path(jsonc_include_dir NAMES json-c/json.h PATHS "${CMAKE_BINARY_DIR}/install/include/")
find_library(jsonc_libs NAMES libjson-c.a PATHS "${CMAKE_BINARY_DIR}/install/lib")

set(Jsonc_LIBRARIES ${jsonc_libs})
set(Jsonc_INCLUDE_DIRS ${jsonc_include_dir})
message(STATUS "==> ${Jsonc_LIBRARIES}")