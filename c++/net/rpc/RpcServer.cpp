#include "RpcServer.h"
#include "base/Logging.h"
using namespace rpc;

RpcServer::RpcServer(EventLoop *loop, const InetAddress &addr)
    : m_pLoop(loop)
    , m_tcpServer(new TcpServer(loop, addr, "RpcServer")) {

    m_tcpServer->SetConnectionCallback([this](TcpConnectionPtr conn) { OnConnection(conn); });

    m_tcpServer->SetMessageCallback([this](const TcpConnectionPtr conn, Buffer *buffer, Timestamp recvTime) { OnMessage(conn, buffer, recvTime); });
}

bool RpcServer::InitEx(int threadNum) {

    m_tcpServer->SetThreadNum(threadNum);
    return true;
}

void RpcServer::OnConnection(TcpConnectionPtr conn) {
}

void RpcServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp recvTime) {
    logger.info("onMessage ==>");
}

void RpcServer::Start() {
    m_tcpServer->Start();
}