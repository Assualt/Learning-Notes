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
    z
    pthread
    ${CMAKE_SOURCE_DIR}/third_party/url/lib/liburlparse.a
)