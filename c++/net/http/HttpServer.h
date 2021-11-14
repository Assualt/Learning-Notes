#pragma once

#include "HttpConfig.h"
#include "base/nonecopyable.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
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
    void setRequestCallBack(CallBack cb);
    void setThreadNum(int num);
    void start();

protected:
    EventLoop *                m_pLoop{nullptr};
    HttpConfig                 m_hConfig;
    CallBack                   m_RequestCallBack;
    std::shared_ptr<TcpServer> m_pServer{nullptr};
};
