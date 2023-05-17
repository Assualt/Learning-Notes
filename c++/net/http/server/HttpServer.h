#pragma once

#include "CtlScanner.h"
#include "WsServer.h"
#include "base/nonecopyable.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"
#include "net/TcpServer.h"
#include "net/http/HttpConfig.h"
#include "net/http/HttpParttern.h"
#include <functional>

using namespace muduo::base;
using namespace muduo::net;

class HttpRequest;
class HttpResponse;
class HttpLog;

using CallBack       = std::function<void(const HttpRequest &, HttpResponse &)>;
using SignalCallback = std::function<void(int, uintptr_t)>;
using SigHandleMap   = std::map<int, std::pair<SignalCallback, uintptr_t>>;

class HttpServer : nonecopyable {
public:
    HttpServer(EventLoop *loop, const InetAddress &address, bool useHttps = false);

    explicit HttpServer(EventLoop *loop, const std::string &cfgPath);

private:
    void Init(EventLoop *loop, bool useSSL);

public:
    void SetRequestCallBack(CallBack cb);

    void onConnect(const TcpConnectionPtr &conn);

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp recvTime);

    void onRequest(const TcpConnectionPtr &conn, HttpRequest &req);

    void SetThreadNum(int num);

    void Start();

    void Exit();

    static RequestMapper &getMapper() { return m_mapper; }

    void RegSignalCallback(int sig, uintptr_t param, SignalCallback cb);

    void startScannerTask(const std::string &libsPath);

private:
    static void SignalHandler(int sig);

protected:
    std::shared_ptr<EventLoop> m_pLoop{nullptr};
    HttpConfig                 m_config;
    CallBack                   m_requestCallBack;
    std::shared_ptr<TcpServer> m_pServer{nullptr};
    static RequestMapper       m_mapper;
    std::shared_ptr<HttpLog>   m_httpLog{nullptr};
    static SigHandleMap        m_signalCallBack;
    ControllerScanner          m_ctlScannerTask;
    std::shared_ptr<WsServer>  m_pWsServer{nullptr};
};
