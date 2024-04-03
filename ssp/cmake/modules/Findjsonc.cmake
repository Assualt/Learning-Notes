include(ExternalProject)
set(jsonc_ROOT         ${PROJECT_SOURCE_DIR}/third_party/json-c)
find_path(JSONC_INCLUDE_DIRS NAMES json-c/json.h PATHS "${PROJECT_SOURCE_DIR}/third_party/install/include/")
find_library(JSONC_LIBRARIES NAMES libjson-c.a PATHS "${PROJECT_SOURCE_DIR}/third_party/install/lib")

if (JSONC_INCLUDE_DIRS AND JSONC_LIBRARIES)
    set(jsonc_FOUND TRUE)
    message(STATUS "!!!****JSONC FOUND****!!!!")
    message(STATUS "JSONC_INCLUDE_DIRS ==> ${JSONC_INCLUDE_DIRS}")
    message(STATUS "JSONC_LIBRARIES ==> ${JSONC_LIBRARIES}")
    return()
endif()

message(STATUS "JSONC_INCLUDE_DIRS ==> ${JSONC_INCLUDE_DIRS}")
message(STATUS "JSONC_LIBRARIES ==> ${JSONC_LIBRARIES}")

message(STATUS "jsonc not found! and begin to configure it")
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E remove_directory ${jsonc_ROOT}/build)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory ${jsonc_ROOT}/build)
EXECUTE_PROCESS(COMMAND cmake -D CMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/third_party/install/ -DBUILD_TESTING=OFF -DBUILD_APPS=OFF ..
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

find_path(JSONC_INCLUDE_DIRS NAMES json-c/json.h PATHS "${PROJECT_SOURCE_DIR}/third_party/install/include/")
find_library(JSONC_LIBRARIES NAMES libjson-c.a PATHS "${PROJECT_SOURCE_DIR}/third_party/install/lib")
set(jsonc_FOUND TRUE)
