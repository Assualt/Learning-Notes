set(test_base_files
    test_object.cpp
    test_logging.cpp
    test_system.cpp
    test_timestamp.cpp
)

add_executable(test_ssp_base ${test_base_files})

target_link_libraries(test_ssp_base
    gtest gtest_main gmock gmock_main
    ssp_base
)

add_test(NAME ssp_base_test COMMAND test_ssp_base)