#pragma once

#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/EventLoopThreadPool.h"
#include "net/TcpServer.h"
#include <memory>

using namespace muduo::net;

namespace rpc {
class RpcServer {
public:
    RpcServer(EventLoop *eventLoop, const InetAddress &addr, bool useSSL);

    bool initEx(int threadNum);

    void start();

    void onConnection(const TcpConnectionPtr &conn);

    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp recvTime);

private:
    EventLoop                 *m_pLoop{nullptr};
    std::unique_ptr<TcpServer> m_tcpServer{nullptr};

    std::unique_ptr<Acceptor> acceptor; // avoid revealing Acceptor
};
} // namespace rpc