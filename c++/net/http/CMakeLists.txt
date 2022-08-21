set(http_common
    HttpConfig.cpp
    HttpContext.cpp
    HttpLog.cpp
    HttpParttern.cpp
    HttpRequest.cpp
    HttpResponse.cpp
    HttpUtils.cpp
)
add_library(http_common SHARED ${http_common})
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(cpp12306)
add_subdirectory(tldextract)