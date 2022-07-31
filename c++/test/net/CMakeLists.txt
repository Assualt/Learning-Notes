add_executable(test_poller test_poller.cpp)
target_link_libraries(test_poller gtest muduo_base muduo_net pthread z)

add_executable(test_tcpclient test_tcpclient.cpp)
target_link_libraries(test_tcpclient gtest muduo_base muduo_net pthread z)

add_test(NAME test_tcp COMMAND test_tcpclient)