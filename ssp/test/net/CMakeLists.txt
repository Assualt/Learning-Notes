set(test_net_objects
    test_socket.cpp
    test_uri.cpp
)
add_executable(test_ssp_net ${test_net_objects})

target_link_libraries(test_ssp_net
PRIVATE
    gtest gtest_main gmock gmock_main
    ssp_base ssp_net
)

add_test(NAME test_ssp_net COMMAND test_ssp_net)