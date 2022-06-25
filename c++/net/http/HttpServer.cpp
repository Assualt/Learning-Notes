#include "HttpServer.h"
#include "HttpContext.h"
#include "HttpLog.h"
#include "HttpUtils.h"
#include "base/Logging.h"
#include "signal.h"
#include <any>
#include <backtrace.h>
#include <functional>

SigHandleMap  HttpServer::m_signalCallBack;
RequestMapper HttpServer::m_mapper;
HttpServer::HttpServer(EventLoop *loop, const InetAddress &addr)
    : m_pLoop(loop)
    , m_pServer(new TcpServer(loop, addr, "tcpServer")) {
    auto &accessLogger = Logger::getLogger("AccessLogger");
    accessLogger.BasicConfig(LogLevel::Debug, "%(message)", "access.log", "");
    m_httpLog.reset(new HttpLog(accessLogger));
    m_pServer->SetConnectionCallback([ this ](TcpConnectionPtr conn) { onConnect(conn); });
    m_pServer->SetMessageCallback([ this ](const TcpConnectionPtr conn, Buffer *buffer, Timestamp stamp) { onMessage(conn, buffer, stamp); });
    m_httpLog->Init();
    m_hConfig.Init("/home/xhou");
}

void HttpServer::SetRequestCallBack(CallBack callback) {
    m_requestCallBack = callback;
}

void HttpServer::onConnect(const TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        logger.info("connection is ready. %s", conn->peerAddress().toIpPort());
    }
}

void HttpServer::SetThreadNum(int num) {
    if (num <= 0) {
        logger.alert("threadnum(<=0) is not allowed ");
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
    if (context.gotAll()) {
        onRequest(conn, context.request());
        context.reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, HttpRequest &request) {
    const string &connection = request.get("Connection");
    bool          close      = true;
    if (strcasecmp(connection.c_str(), "keep-alive") == 0) {
        close = false;
    }
    HttpResponse response(close);
    Func         func = m_mapper.find(request.getRequestPath(), request.getRequestType(), request.GetHeaderMap());

    std::string basicRequestPath = m_hConfig.getServerRoot();
    if (basicRequestPath.back() != '/') {
        basicRequestPath += "/";
    }

    if (request.getRequestFilePath().front() == '/') {
        basicRequestPath += request.getRequestFilePath().substr(1);
    } else {
        basicRequestPath += request.getRequestFilePath();
    }

    bool fileExists = utils::FileExists(basicRequestPath);
    if (!fileExists) {
        func(request, response, m_hConfig);
    } else if (utils::IsDir(basicRequestPath)) { // 展示目录
        func = m_mapper.find(DefaultPattern, request.getRequestType());
        func(request, response, m_hConfig);
    } else {
        func = m_mapper.find(FilePattern, request.getRequestType());
        func(request, response, m_hConfig);
    }

    Buffer buf;
    response.appendToBuffer(buf);
    conn->send(&buf);

    if (true) {
        (*m_httpLog) << conn->localAddress().toIpPort() << " - [" << utils::requestTimeFmt() << "] \"" << request.getRequestType() << " " << request.getRequestPath() << " " << request.getHttpVersion()
                     << "\" " << response.getStatusCode() << " " << buf.readableBytes() << " \"" << request.get(UserAgent) << "\"" << CTRL;
    }
    if (response.closeConnection()) {
        conn->shutdown();
    }
}

void HttpServer::Start() {
    logger.info("HttpServer start ... at:%s", m_pServer->IpPort());
    m_pServer->Start();
}

void HttpServer::Exit() {
    logger.info("HttpServer stop ... at:%s", m_pServer->IpPort());
    m_pServer->Stop();
}

void HttpServer::RegSignalCallback(int sig, uintptr_t param, SignalCallback cb) {
    if (cb == nullptr) {
        logger.warning("cb for sig %d is null, skip to add signal", sig);
        return;
    }

    if (m_signalCallBack.count(sig) != 0) {
        logger.warning("repeated to reg sig %d cb.", sig);
        return;
    }

    m_signalCallBack[ sig ] = {cb, param};
    signal(sig, HttpServer::SignalHandler);
}

void HttpServer::SignalHandler(int sig) {
    if (m_signalCallBack.count(sig) == 0) {
        logger.info("no sig handle register for sig:%d", sig);
        return;
    }

    LOG_SYSTEM.warning("begin to execute sig %d handler..", sig);
    m_signalCallBack[ sig ].first(sig, m_signalCallBack[ sig ].second);
}