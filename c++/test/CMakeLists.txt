cmake_minimum_required(VERSION 2.6)
project(test)

set(src_app
    unitmain.cpp
    main.cpp
)
add_executable(unitmain ${src_app})
target_link_libraries(unitmain gtest)
add_test(NAME unitmain COMMAND ${PROJECT_NAME})
add_subdirectory(base)
add_subdirectory(net)