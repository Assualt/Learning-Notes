//
// Created by xhou on 2022/9/22.
//

#include "WsServer.h"
#include "base/Logging.h"
#include "net/TcpConnection.h"

WsServer::WsServer(EventLoop *loop, const InetAddress &address, const HttpConfig &cfg, bool useHttps)
    : server_(new TcpServer(loop, address, "WsServer", useHttps)) {

    server_->SetThreadNum(cfg.getWebSocketThreadNum());
    server_->SetConnectionCallback([ this ](TcpConnectionPtr conn) { onConnect(conn); });
    server_->SetMessageCallback(
        [ this ](const TcpConnectionPtr conn, Buffer *buffer, Timestamp stamp) { onMessage(conn, buffer, stamp); });
}

void WsServer::start() {
    logger.info("HttpServer start ... at:%s", server_->IpPort());
    server_->Start();
}

void WsServer::stop() {
    logger.info("WsServer stop ... at:%s", server_->IpPort());
    server_->Stop();
}

void WsServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp recvTime) {
    logger.info("recv time buf %s", buf->peek());

    conn->send(buf->peek(), buf->readableBytes());
    conn->shutdown();
}

void WsServer::onConnect(const TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        logger.info("[WS/WSS]connection is ready. %s", conn->peerAddress().toIpPort());
    }
}

void WsServer::onRequest(const TcpConnectionPtr &conn) {}