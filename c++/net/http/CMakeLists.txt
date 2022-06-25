set(http_SRCS
    HttpServer.cpp
    HttpConfig.cpp
    HttpContext.cpp
    HttpLog.cpp
    HttpRequest.cpp
    HttpResponse.cpp
    HttpPattern.cpp
    HttpUtils.cpp
    controller/DefaultController.cpp
    controller/40xController.cpp
    main_server.cpp
)
add_executable(httpserver ${http_SRCS})
target_link_libraries(httpserver muduo_base muduo_net pthread magic z)
install(TARGETS httpserver DESTINATION bin)

file(GLOB HEADERS_base  "*.h")
install(FILES ${HEADERS_base} DESTINATION include/net/http)