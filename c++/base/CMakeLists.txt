set(base_SRCS
    Backtrace.cpp
    Condition.cpp
    Configure.cpp
    CountDownLatch.cpp
    Date.cpp
    Dll.cpp
    DirScanner.cpp
    File.cpp
    Logging.cpp
    Mutex.cpp
    Thread.cpp
    ThreadPool.cpp
    Timestamp.cpp
    json/json.cpp
    stream/StreamBase.cpp
)

add_library(muduo_base ${base_SRCS})
target_link_libraries(muduo_base pthread rt)

install(TARGETS muduo_base DESTINATION lib)

file(GLOB HEADERS_base  "*.h")
install(FILES ${HEADERS_base} DESTINATION include/base)

file(GLOB HEADERS_stream "stream/*.h")
install(FILES ${HEADERS_stream} DESTINATION include/base/stream)