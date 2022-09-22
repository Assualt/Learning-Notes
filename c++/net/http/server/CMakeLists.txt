set(http_server_objects
    CtlScanner.cpp
    HttpServer.cpp
    WsServer.cpp
    main_server.cpp
    controller/DefaultController.cpp
    controller/40xController.cpp
    controller/UserController.cpp
    controller/WebSockController.cpp
)

add_subdirectory(controller/lib)
add_executable(httpserver ${http_server_objects})
target_link_libraries(httpserver 
    muduo_base 
    muduo_net 
    pthread 
    magic 
    z 
    dl
    http_common
)