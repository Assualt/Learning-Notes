include(ExternalProject)
set(openssl_ROOT ${PROJECT_SOURCE_DIR}/third_party/openssl)

set(Postfix "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(Postfix ".so")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(Postfix ".so")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(Postfix ".dylib")
endif ()

find_path(OPENSSL_INCLUDE_DIRS NAMES openssl/ssl.h PATHS "${PROJECT_SOURCE_DIR}/third_party/install/include")
find_library(SSL_LIB NAMES libssl${Postfix} PATHS "${PROJECT_SOURCE_DIR}/third_party/install/lib")
find_library(CRYPTO_LIB NAMES libcrypto${Postfix} PATHS "${PROJECT_SOURCE_DIR}/third_party/install/lib")
if (OPENSSL_INCLUDE_DIRS AND SSL_LIB AND CRYPTO_LIB)
    set(OPENSSL_FOUND TRUE)
    message(STATUS "!!!****OPENSSL FOUND****!!!")
    set(OPENSSL_LIBRARIES ${SSL_LIB} ${CRYPTO_LIB})
    message(STATUS "OPENSSL_INCLUDE_DIRS ===> ${OPENSSL_INCLUDE_DIRS}")
    message(STATUS "OPENSSL_LIBRARIES ===> ${OPENSSL_LIBRARIES}")
    return()
endif ()

message(STATUS "openssl not found and begin to configure it")
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove_directory ${openssl_ROOT}/build)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${openssl_ROOT}/build)
EXECUTE_PROCESS(
    COMMAND ../Configure --prefix=${PROJECT_SOURCE_DIR}/third_party/install/ --libdir=${PROJECT_SOURCE_DIR}/third_party/install/lib shared
    WORKING_DIRECTORY ${openssl_ROOT}/build
)

EXECUTE_PROCESS(
    COMMAND make build_libs -j8
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE build_result
    WORKING_DIRECTORY ${openssl_ROOT}/build
)

if (build_result)
    message(FATAL_ERROR "openssl configure failed")
endif ()

EXECUTE_PROCESS(
    COMMAND make install_sw -j16
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE build_result
    WORKING_DIRECTORY ${openssl_ROOT}/build
)

if (build_result)
    message(FATAL_ERROR "openssl build error")
endif ()

find_path(OPENSSL_INCLUDE_DIRS NAMES openssl/ssl.h PATHS "${PROJECT_SOURCE_DIR}/third_party/install/include")
find_library(SSL_LIB NAMES libssl${Postfix} PATHS "${PROJECT_SOURCE_DIR}/third_party/install/lib")
find_library(CRYPTO_LIB NAMES libcrypto${Postfix} PATHS "${PROJECT_SOURCE_DIR}/third_party/install/lib")
set(OPENSSL_LIBRARIES ${SSL_LIB} ${CRYPTO_LIB})
set(OPENSSL_FOUND TRUE)