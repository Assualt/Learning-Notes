include(ExternalProject)
message(STATUS "begin to find openssl")

set(openssl_ROOT         ${PROJECT_SOURCE_DIR}/third_party/openssl)
# 指定配置指令（注意此处修改了安装目录，否则默认情况下回安装到系统目录）
set(openssl_CONFIGURE    cd ${openssl_ROOT}/ && mkdir install && cmake -D CMAKE_INSTALL_PREFIX=${openssl_ROOT}/install .)
# 指定编译指令（需要覆盖默认指令，进入我们指定的openssl_ROOT目录下）
set(openssl_MAKE         cd ${openssl_ROOT}/ && make)
# 指定安装指令（需要覆盖默认指令，进入我们指定的openssl_ROOT目录下）
set(openssl_INSTALL      cd ${openssl_ROOT}/ && make install)

ExternalProject_Add(GoogleTest
    PREFIX            ${openssl_ROOT}
    SOURCE_DIR        ${openssl_ROOT}
    CONFIGURE_COMMAND ${openssl_CONFIGURE}
    BUILD_COMMAND     ${openssl_MAKE}
    INSTALL_COMMAND   ${opensll_INSTALL}
)

find_path(OPENSSL_INCLUDE_DIR
NAMES
    openssl/ssl.h
PATHS
    ${openssl_ROOT}/install
)