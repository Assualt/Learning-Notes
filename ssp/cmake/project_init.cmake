set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

message(STATUS "OUTPUT PATH: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "ARCHIVE OUTPUT PATH: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(jsonc REQUIRED)
find_package(OpenSSL)
if (OPENSSL_FOUND)
    message(STATUS "open ssl is found ${OPENSSL_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "open ssl is not found ${OPENSSL_INCLUDE_DIR}")
endif ()