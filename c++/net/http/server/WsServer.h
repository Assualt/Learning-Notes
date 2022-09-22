//
// Created by xhou on 2022/9/22.
//

#ifndef MUDUO_BASE_TOOLS_WSSERVER_H
#define MUDUO_BASE_TOOLS_WSSERVER_H

#include "net/EventLoop.h"
#include "net/TcpServer.h"
#include "net/http/HttpConfig.h"
using namespace muduo::net;
class WsServer {
public:
    WsServer(EventLoop *loop, const InetAddress &address, const HttpConfig &cfg, bool useHttps = false);

    ~WsServer() = default;

    void start();

    void stop();

private:
    void onConnect(const TcpConnectionPtr &conn);

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp recvTime);

    void onRequest(const TcpConnectionPtr &conn);

private:
    std::shared_ptr<TcpServer> server_{nullptr};

    EventLoop loop_;
};

#endif // MUDUO_BASE_TOOLS_WSSERVER_H
