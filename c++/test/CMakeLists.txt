cmake_minimum_required(VERSION 3.13.0)
project(test)

set(src_app
    main.cpp
    unitmain.cpp
)
add_executable(unitmain ${src_app})
target_link_libraries(unitmain gtest)
add_test(NAME unitmain COMMAND unitmain)
add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(db)