set(muduo_base_objects
    Backtrace.cpp
    Condition.cpp
    Configure.cpp
    Conv.cpp
    CountDownLatch.cpp
    Date.cpp
    Dll.cpp
    DirScanner.cpp
    File.cpp
    Logging.cpp
    Mutex.cpp
    StringUtil.cpp
    Timer.cpp
    Thread.cpp
    ThreadPool.cpp
    Timestamp.cpp
    ProcInfo.cpp
    checksum/checksum.cpp
    Unicode.cpp
    crypto/base64.cpp
    json/json.cpp
    stream/StreamBase.cpp
    stream/BinaryStream.cpp
        LogHandle.cpp)

add_subdirectory(fsm)

add_library(muduo_base SHARED ${muduo_base_objects})
target_link_libraries(muduo_base pthread rt dl)

install(TARGETS muduo_base DESTINATION lib)

file(GLOB HEADERS_base "*.h")
install(FILES ${HEADERS_base} DESTINATION include/base)

file(GLOB HEADERS_stream "stream/*.h")
install(FILES ${HEADERS_stream} DESTINATION include/base/stream)