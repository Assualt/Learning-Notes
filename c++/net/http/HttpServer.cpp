#include "HttpServer.h"
#include "HttpContext.h"
#include "base/Logging.h"
#include <any>
#include <functional>
HttpServer::HttpServer(EventLoop *loop, const InetAddress &addr)
    : m_pLoop(loop)
    , m_pServer(new TcpServer(loop, addr, "tcpServer")) {
    m_pServer->setConnectionCallback(std::bind(&HttpServer::onConnect, this, std::placeholders::_1));
    m_pServer->setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req) {
    const string &connection = req.get("Connection");
    bool          close      = connection == "close" || (req.getHttpVersion() == "HTTP/1.0" && connection != "Keep-Alive");
    HttpResponse  response(close);
    m_RequestCallBack(req, response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection()) {
        conn->shutdown();
    }
}

void HttpServer::start() {
    logger.info("HttpServer start ...");
    m_pServer->start();
}
