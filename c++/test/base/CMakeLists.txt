add_executable(test_logging test_logging.cpp)
target_link_libraries(test_logging gtest muduo_base dl)
add_test(NAME test_logging COMMAND test_logging)

add_executable(test_range test_range.cpp)
target_link_libraries(test_range gtest muduo_base dl)
add_test(NAME test_range COMMAND test_range)

add_executable(test_thread test_thread.cpp)
target_link_libraries(test_thread gtest muduo_base dl)
add_test(NAME test_thread COMMAND test_thread)

add_executable(test_timestamp test_timestamp.cpp)
target_link_libraries(test_timestamp gtest muduo_base dl)
add_test(NAME test_timestamp COMMAND test_timestamp)

add_executable(test_threadpool test_threadpool.cpp)
target_link_libraries(test_threadpool gtest muduo_base dl)
add_test(NAME test_threadpool COMMAND test_threadpool)

add_executable(test_file test_file.cpp)
target_link_libraries(test_file gtest muduo_base dl)
add_test(NAME test_file COMMAND test_file)

add_executable(test_checksum test_checksum.cpp)
target_link_libraries(test_checksum gtest muduo_base dl)
add_test(NAME test_checksum COMMAND test_checksum)

add_executable(test_crypto test_crypto.cpp)
target_link_libraries(test_crypto gtest muduo_base dl)
add_test(NAME test_crypto COMMAND test_crypto)

add_executable(test_stream test_stream.cpp)
target_link_libraries(test_stream gtest muduo_base dl)
add_test(NAME test_stream COMMAND test_stream)

add_executable(test_timer test_timer.cpp)
target_link_libraries(test_timer muduo_base dl)
add_test(NAME test_timer COMMAND test_timer)

install(TARGETS test_logging DESTINATION bin)
install(TARGETS test_range DESTINATION bin)
install(TARGETS test_thread DESTINATION bin)
install(TARGETS test_timestamp DESTINATION bin)
install(TARGETS test_threadpool DESTINATION bin)
install(TARGETS test_file DESTINATION bin)

