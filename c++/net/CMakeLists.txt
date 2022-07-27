set(muduo_net_objects
    Acceptor.cpp
    Buffer.cpp
    Channel.cpp
    CurrentThread.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    InetAddress.cpp
    Poller.cpp
    Socket.cpp
    SocketsOp.cpp
    TcpServer.cpp
    TcpConnection.cpp
    TcpConnection.cpp
    poller/EPollPoller.cpp
    poller/PollPoller.cpp
)


add_library(muduo_net SHARED ${muduo_net_objects})
target_link_libraries(muduo_net muduo_base pthread rt ssl crypto)
install(TARGETS muduo_net DESTINATION lib)

file(GLOB HEADERS_net
    "*.h"
    "poller/*.h"
)
install(FILES ${HEADERS_base} DESTINATION include/net)

add_subdirectory(http)
add_subdirectory(rpc)