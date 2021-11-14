add_executable(test_poller test_poller.cpp)
target_link_libraries(test_poller muduo_base muduo_net pthread)
add_test(NAME test_poller COMMAND test_poller)
