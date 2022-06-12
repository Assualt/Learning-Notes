cmake_minimum_required(VERSION 2.6)
project(test)

set(src_app
    unitmain.cpp
    main.cpp
)
add_executable(${PROJECT_NAME} ${src_app})
target_link_libraries(${PROJECT_NAME} gtest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
add_subdirectory(base)
add_subdirectory(net)