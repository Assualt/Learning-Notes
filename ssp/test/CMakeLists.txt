add_executable(main main.cpp)

target_link_libraries(main
PRIVATE
    gtest gtest_main gmock gmock_main
)

add_test(NAME main COMMAND main)
add_subdirectory(base)
add_subdirectory(net)