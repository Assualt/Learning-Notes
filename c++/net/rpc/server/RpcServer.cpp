#include "RpcServer.h"
#include "base/Logging.h"
#include "base/ProcInfo.h"
#include "net/TcpConnection.h"
using namespace rpc;

RpcServer::RpcServer(EventLoop *loop, const InetAddress &addr, bool useSSL)
    : m_pLoop(loop)
    , m_tcpServer(new TcpServer(loop, addr, "RpcServer", useSSL)) {
    m_tcpServer->SetConnectionCallback([ this ](const TcpConnectionPtr &conn) { onConnection(conn); });
    m_tcpServer->SetMessageCallback([ this ](const TcpConnectionPtr &conn, Buffer *buffer, Timestamp recvTime) {
        onMessage(conn, buffer, recvTime);
    });
}

bool RpcServer::initEx(int threadNum) {
    m_tcpServer->SetThreadNum(threadNum);
    return true;
}

void RpcServer::onConnection(const TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        logger.info("connection is ready. %s", conn->peerAddress().toIpPort());
    }
}

void RpcServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp recvTime) {
    if (buffer == nullptr) {
        conn->shutdown();
        return;
    }

    logger.info("recv buffer is ==> %s <==", buffer->peek());
    Buffer out;
    out.append("{\"name\":\"OK\"}");

    conn->send(&out);
    conn->shutdown();
}

void RpcServer::start() {
    m_tcpServer->Start();
    logger.info("RpcServer start ... at:%s", m_tcpServer->IpPort());
}