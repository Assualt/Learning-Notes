#pragma once

#include "HttpConfig.h"
#include "HttpParttern.h"
#include "base/nonecopyable.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"
#include "net/TcpServer.h"
#include <functional>

using namespace muduo::base;
using namespace muduo::net;

class HttpRequest;
class HttpResponse;

using CallBack = std::function<void(const HttpRequest &, HttpResponse &)>;

class HttpServer : nonecopyable {
public:
    HttpServer(EventLoop *loop, const InetAddress &addr);

public:
    void                  setRequestCallBack(CallBack cb);
    void                  onConnect(const TcpConnectionPtr &conn);
    void                  onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp recvTime);
    void                  onRequest(const TcpConnectionPtr &conn, HttpRequest &req);
    void                  setThreadNum(int num);
    void                  start();
    inline RequestMapper &getMapper() {
        return m_mapper;
    }

protected:
    EventLoop *                m_pLoop{nullptr};
    HttpConfig                 m_hConfig;
    CallBack                   m_RequestCallBack;
    std::shared_ptr<TcpServer> m_pServer{nullptr};
    RequestMapper              m_mapper;
};
