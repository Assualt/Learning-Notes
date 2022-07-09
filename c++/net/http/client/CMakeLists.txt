set(client_source
    HttpClient.cpp
    main_client.cpp
)

add_executable(httpclient ${client_source})
target_link_libraries(httpclient 
    muduo_base
    muduo_net
    z
    pthread
)