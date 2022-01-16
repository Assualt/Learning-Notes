#include "HttpServer.h"
#include "HttpContext.h"
#include "HttpLog.h"
#include "HttpUtils.h"
#include "base/Logging.h"
#include <any>
#include <backtrace.h>
#include <functional>

HttpServer::HttpServer(EventLoop *loop, const InetAddress &addr)
    : m_pLoop(loop)
    , m_pServer(new TcpServer(loop, addr, "tcpServer")) {
    
    auto &accessLogger = Logger::getLogger("AccessLogger");
    accessLogger.BasicConfig(Logger::Debug, "%(message)", "access.log", "");
    m_httpLog.reset(new HttpLog(accessLogger));
    m_pServer->setConnectionCallback(std::bind(&HttpServer::onConnect, this, std::placeholders::_1));
    m_pServer->setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_httpLog->Init();
    m_hConfig.Init("/home/xhou");
}

void HttpServer::setRequestCallBack(CallBack callback) {
    m_RequestCallBack = callback;
}

void HttpServer::onConnect(const TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
    }
}

void HttpServer::setThreadNum(int num) {
    if (num <= 0) {
        logger.alert("threadnum(<=0) is not allowed ");
        return;
    }
    m_pServer->setThreadNum(num);
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp recvTime) {
    HttpContext context;
    if (!context.parseRequest(buf, recvTime)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n", 32);
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
    Func         func = m_mapper.find(request.getRequestPath(), request.getRequestType());
    
    std::string basicRequestPath = m_hConfig.getServerRoot();
    if (basicRequestPath.back() != '/')
        basicRequestPath += "/";
    if (request.getRequestFilePath().front() == '/')
        basicRequestPath += request.getRequestFilePath().substr(1);
    else
        basicRequestPath += request.getRequestFilePath();

    bool fileExists = utils::FileExists(basicRequestPath);
    if (!fileExists) {
        func(request, response, m_hConfig);
    } else if (utils::IsDir(basicRequestPath)) {
        func = m_mapper.find("/#//", request.getRequestType());
        func(request, response, m_hConfig);
    } else {
        func = m_mapper.find("/#/", request.getRequestType());
        func(request, response, m_hConfig);
    }

    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);

    if (true) {
        (*m_httpLog) << conn->localAddress().toIpPort() << " - [" << utils::requstTimeFmt() << "] \"" << request.getRequestType() << " " << request.getRequestPath() << " "
                    << request.getHttpVersion() << "\" " << response.getStatusCode() << " " << buf.readableBytes() << " \"" << request.get(UserAgent) << "\"\n";
    }
    if (response.closeConnection()) {
        conn->shutdown();
    }
}

void HttpServer::start() {
    logger.info("HttpServer start ... at:%s", m_pServer->ipPort());
    m_pServer->start();
}
