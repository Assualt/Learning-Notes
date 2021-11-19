set(http_SRCS
    HttpServer.cpp
    HttpConfig.cpp
    HttpContext.cpp
    HttpRequest.cpp
    HttpResponse.cpp
    main_server.cpp
)
add_executable(httpserver ${http_SRCS})
target_link_libraries(httpserver muduo_base muduo_net pthread)


