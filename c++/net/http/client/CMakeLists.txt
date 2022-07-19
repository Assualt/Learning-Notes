set(client_source
    HttpClient.cpp
    HttpConnection.cpp
    main_client.cpp
)

add_executable(httpclient ${client_source})
message(STATUS ${CMAKE_SOURCE_DIR})
target_link_libraries(httpclient
    muduo_base
    muduo_net
    http_common
    z
    pthread
    magic
    ${CMAKE_SOURCE_DIR}/third_party/url/lib/liburlparse.a
)

add_test(
    NAME
        test_client
    COMMAND
        httpclient --url http://127.0.0.1:8100/login/test/xhou?query=123 --verbose 0
)

add_test(
    NAME
        test_client_baidu.com
    COMMAND
        httpclient --url https://www.jianshu.com/p/7ce037a2eafb --verbose 1
)