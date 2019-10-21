cmake_minimum_required(VERSION 3.10)
project(mxt)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/${PROJECT_NAME})
set(INC_DIR ${SOURCE_DIR}/include)
SET(LIBRARY_OUTPUT_PATH ${SOURCE_DIR}/lib)

aux_source_directory(${SOURCE_DIR}/src/base SRC_BASE_LIST)
aux_source_directory(${SOURCE_DIR}/src/ SRC_CONF_LIST)
aux_source_directory(${SOURCE_DIR}/src/ SRC_SYS_LIST)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})

#生成静态库
add_library(mxt STATIC ${SRC_BASE_LIST} ${SRC_CONF_LIST} ${SRC_SYS_LIST})

set(TEST_DIR ${SOURCE_DIR}/test)

#CFLAGS
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
#TEST
add_executable(test_config ${TEST_DIR}/test_tconfig.cpp)
#add_executable(${PROJECT_NAME}_exec ${TEST_DIR}/test.cpp)
##
message(STATUS "${LIBRARY_OUTPUT_PATH} ")
#target_link_libraries(test_config ${LIBRARY_OUTPUT_PATH}/libmxt.a)

