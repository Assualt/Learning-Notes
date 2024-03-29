#include "HttpServer.h"
#include "base/Backtrace.h"
#include "base/Logging.h"
#include "base/System.h"
#include "controller/Controller_if.h"
#include "net/http/HttpContext.h"
#include "net/http/HttpLog.h"
#include "net/http/HttpUtils.h"
#include <any>
#include <functional>
#include <utility>

SigHandleMap  HttpServer::m_signalCallBack;
RequestMapper HttpServer::m_mapper;

HttpServer::HttpServer(EventLoop *loop, const InetAddress &address, bool useHttps)
    : m_pLoop(loop)
    , m_pServer(new TcpServer(loop, address, "tcpServer")) {
    Init(loop, useHttps);
}

HttpServer::HttpServer(EventLoop *loop, const std::string &cfgPath)
    : m_pLoop(loop) {
    m_config.Init(cfgPath);
    m_pServer = std::make_shared<TcpServer>(loop, InetAddress(m_config.getServerPort()), m_config.getServerName());
    Init(loop, m_config.getSupportSSL());
}

void HttpServer::Init(EventLoop *loop, bool useSSl) {
    auto &accessLogger = Logger::getLogger("AccessLogger");
    accessLogger.BasicConfig(LogLevel::Debug, "%(message)", "access.log", "");
    m_httpLog.reset(new HttpLog(accessLogger));
    m_pServer->SetConnectionCallback([this](const TcpConnectionPtr &conn) { onConnect(conn); });
    m_pServer->SetMessageCallback(
        [this](const TcpConnectionPtr &conn, Buffer *buffer, Timestamp stamp) { onMessage(conn, buffer, stamp); });
    m_pServer->SetMsgTimeoutCallback(0, nullptr);
    m_httpLog->Init();
    if (m_config.getSupportWebSocket()) {
        m_pWsServer = std::make_shared<WsServer>(loop, InetAddress(m_config.getWebSocketPort()), m_config,
                                                 m_config.getSupportSSL());
    }

    if (useSSl) {
        m_pServer->InitSslConfig(m_config.getSSLCertPath(), m_config.getSSLPrivateKeyPath());
    }
}

void HttpServer::SetRequestCallBack(CallBack callback) { m_requestCallBack = std::move(callback); }

void HttpServer::onConnect(const TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        logger.info("[HTTP/HTTPS]connection is ready. %s", conn->peerAddress().toIpPort());
    }
}

void HttpServer::SetThreadNum(int num) {
    if (num <= 0) {
        logger.alert("thread num(<=0) is not allowed ");
        return;
    }
    m_pServer->SetThreadNum(num);
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp recvTime) {
    HttpContext context;
    if (!context.parseRequest(buf, recvTime)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n", 29);
        conn->shutdown();
    }

    if (context.gotAll() || context.gotEnd()) {
        onRequest(conn, context.request());
        context.reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, HttpRequest &request) {
    bool close = true;
    if (strcasecmp(request.get(Connection).c_str(), "keep-alive") == 0) {
        close = false;
    }

    HttpResponse response(close);
    std::string  basicRequestPath = m_config.getServerRoot();
    if (basicRequestPath.back() != '/') {
        basicRequestPath += "/";
    }

    if (request.getRequestFilePath().front() == '/') {
        basicRequestPath += request.getRequestFilePath().substr(1);
    } else {
        basicRequestPath += request.getRequestFilePath();
    }

    auto objHandle =
        m_mapper.findHandle(request.getRequestPath(), request.getRequestType(), request.GetRequestQueryMap());
    bool fileExists = utils::FileExists(basicRequestPath);
    if (!fileExists && objHandle.has_value()) {
        reinterpret_cast<IController *>(objHandle.value())->onRequest(request, response, m_config);
    } else if (utils::IsDir(basicRequestPath)) { // 展示目录
        auto obj = m_mapper.findHandle(DefaultPattern, request.getRequestType(), request.GetRequestQueryMap());
        reinterpret_cast<IController *>(obj.value())->onRequest(request, response, m_config);
    } else {
        auto obj = m_mapper.findHandle(FilePattern, request.getRequestType(), request.GetRequestQueryMap());
        auto ret = reinterpret_cast<IController *>(obj.value())->onRequest(request, response, m_config);
        if (!ret) { // 文件不存在
            obj = m_mapper.findHandle(NOTFOUND, request.getRequestType(), request.GetRequestQueryMap());
            reinterpret_cast<IController *>(obj.value())->onRequest(request, response, m_config);
        }
    }

    Buffer buf;
    response.appendToBuffer(buf);
    conn->send(&buf);

    (*m_httpLog) << conn->localAddress().toIpPort() << " - [" << utils::requestTimeFmt() << "] \""
                 << request.getRequestType() << " " << request.getRequestPath() << " " << request.getHttpVersion()
                 << "\" " << response.getStatusCode() << " " << buf.readableBytes() << " \"" << request.get(UserAgent)
                 << "\"" << CTRL;

    if (response.closeConnection()) {
        conn->shutdown();
    }
}

void HttpServer::Start() {
    logger.info("HttpServer start ... at:%s", m_pServer->IpPort());
    m_pServer->Start();
    if (m_pWsServer != nullptr) {
        m_pWsServer->start();
    }
}

void HttpServer::Exit() {
    logger.info("HttpServer stop ... at:%s", m_pServer->IpPort());
    m_pServer->Stop();
    if (m_pWsServer != nullptr) {
        m_pWsServer->stop();
    }
    m_ctlScannerTask.stopTask();
}

void HttpServer::RegSignalCallback(int sig, uintptr_t param, SignalCallback cb) {
    if (cb == nullptr) {
        logger.warning("cb for sig %d is null, skip to add signal", sig);
        return;
    }

    if (m_signalCallBack.find(sig) != m_signalCallBack.end()) {
        logger.warning("repeated to reg sig %d cb.", sig);
        return;
    }

    m_signalCallBack[ sig ] = {cb, param};
    struct sigaction sa {};
    sa.sa_handler = HttpServer::SignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(sig, &sa, nullptr);
}

void HttpServer::SignalHandler(int sig) {
    if (m_signalCallBack.count(sig) == 0) {
        logger.info("no sig handle register for sig:%d", sig);
        return;
    }

    LOG_SYSTEM.warning("begin to execute sig %s handler..", System::GetSigName(sig));
    m_signalCallBack[ sig ].first(sig, m_signalCallBack[ sig ].second);
}

void HttpServer::startScannerTask(const std::string &libsPath) {
    m_ctlScannerTask.init(libsPath);
    m_ctlScannerTask.startTask();
}