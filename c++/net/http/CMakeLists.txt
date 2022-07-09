set(http_common
    HttpConfig.cpp
    HttpContext.cpp
    HttpLog.cpp
    HttpParttern.cpp
    HttpRequest.cpp
    HttpResponse.cpp
    HttpUtils.cpp
)
add_library(http_common ${http_common})
add_subdirectory(server)
add_subdirectory(client)