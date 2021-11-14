add_executable(test_logging test_logging.cpp)
target_link_libraries(test_logging muduo_base)
add_test(NAME test_logging COMMAND test_logging)

add_executable(test_range test_range.cpp)
target_link_libraries(test_range muduo_base)
add_test(NAME test_range COMMAND test_range)

add_executable(test_thread test_thread.cpp)
target_link_libraries(test_thread muduo_base)
add_test(NAME test_thread COMMAND test_thread)

add_executable(test_timestamp test_timestamp.cpp)
target_link_libraries(test_timestamp muduo_base)
add_test(NAME test_timestamp COMMAND test_timestamp)

add_executable(test_threadpool test_threadpool.cpp)
target_link_libraries(test_threadpool muduo_base)
add_test(NAME test_threadpool COMMAND test_threadpool)

add_executable(test_file test_file.cpp)
target_link_libraries(test_file muduo_base)
add_test(NAME test_file COMMAND test_file)


