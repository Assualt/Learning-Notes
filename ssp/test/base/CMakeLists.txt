set(test_base_files
    test_object.cpp
    test_logging.cpp
    test_system.cpp
    test_timestamp.cpp
    test_string_utils.cpp
    test_file.cpp
    test_dll.cpp
    test_dirscanner.cpp
    test_conf.cpp
    test_format.cpp
    test_jsonkit.cpp
)

add_executable(test_ssp_base ${test_base_files})

target_link_libraries(test_ssp_base
    gtest gtest_main gmock gmock_main
    ssp_base
)

add_test(NAME ssp_base_test COMMAND test_ssp_base)

add_executable(test_thread test_thread.cpp)
target_link_libraries(test_thread
    gtest gtest_main gmock gmock_main
    ssp_base
)

add_test(NAME test_thread COMMAND test_thread)

add_executable(test_thread_pool test_threadpool.cpp)
target_link_libraries(test_thread_pool
    gtest gtest_main gmock gmock_main
    ssp_base
)

add_test(NAME test_thread_pool COMMAND test_thread_pool)

add_executable(test_thread_pool_ex test_threadpool_ex.cpp)
target_link_libraries(test_thread_pool_ex
    gtest gtest_main gmock gmock_main
    ssp_base
)

add_test(NAME test_thread_pool_ex COMMAND test_thread_pool_ex)
