#include "HttpServer.h"
#include "base/Logging.h"
HttpServer::HttpServer(EventLoop *loop, const InetAddress &addr)
    : m_pLoop(loop)
    , m_pServer(new TcpServer(loop, addr, "tcpServer")) {
}

void HttpServer::setRequestCallBack(CallBack callback) {
    m_RequestCallBack = callback;
}

void HttpServer::setThreadNum(int num) {
    if (num <= 0) {
        logger.alert("threadnum(<=0) is not allowed ");
        return;
    }
}

void HttpServer::start() {
    logger.info("HttpServer start ...");
    m_pServer->start();
}
