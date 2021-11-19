#include "TcpServer.h"
#include "Acceptor.h"
#include "Buffer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "SocketsOp.h"
#include "TcpConnection.h"
#include "base/Format.h"
#include "base/Logging.h"
#include <functional>
using namespace std;
namespace muduo {
namespace net {
void defaultConnectionCallback(const TcpConnectionPtr &conn) {
    logger.info("Connnection local address:[%s] -> peer address:[%s] ", conn->localAddress().toIpPort(), conn->peerAddress().toIpPort());
}

void defaultMessageCallback(const TcpConnectionPtr &, Buffer *buf, Timestamp) {
    logger.info("message call back ....");
    buf->retrieveAll();
}

void TcpServer::setThreadNum(int threadNum) {
    m_threadPool->setThreadNum(threadNum);
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr, const std::string &serverName, AddressOption option)
    : m_pLoop(loop)
    , m_strServerName(serverName)
    , acceptor(new Acceptor(loop, addr, false))
    , connectionCallback(defaultConnectionCallback)
    , messageCallback(defaultMessageCallback)
    , m_threadPool(new EventLoopThreadPool(loop, serverName))
    , m_nNexcConnId(1) {
    acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddress) {
    m_pLoop->assertLoopThread();
    auto        ioLoop = m_threadPool->getNextLoop();
    InetAddress locAddr(sockets::getLocalAddr(sockfd));
    char        buf[ 64 ];
    ++m_nNexcConnId;
    snprintf(buf, sizeof buf, "-%s#%d", locAddr.toIpPort().c_str(), m_nNexcConnId);
    std::string connName = FmtString("%-%").arg(m_strServerName).arg(buf).str();
    logger.info("TcpServer::newConnection [%s] - new connection [ %s ] from [%s]", m_strServerName, connName, peerAddress.toIpPort());
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, locAddr, peerAddress));
    conn->setConnectionCallBack(connectionCallback);
    conn->setMessageCallBack(messageCallback);
    conn->setCloseCallBack(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    connectionMap[ connName ] = conn;
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    m_pLoop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    m_pLoop->assertLoopThread();
    logger.info("TcpServer::remoteConnectionInLoop name:%s - connection Name:%s", m_strServerName, conn->name());
    size_t     n    = connectionMap.erase(conn->name());
    EventLoop *loop = conn->getLoop();
    loop->queueInLoop(std::bind(&TcpConnection::connectDestory, conn));
}

void TcpServer::setConnectionCallback(const ConnectionCallback &callback) {
    connectionCallback = callback;
}

void TcpServer::setMessageCallback(const MessageCallback &callback) {
    messageCallback = callback;
}

void TcpServer::setThreadInitCallback(const ThreadInitCallback &callback) {
    threadInitCallback = callback;
}

void TcpServer::start() {
    m_threadPool->start(threadInitCallback);
    m_pLoop->runInLoop(std::bind(&Acceptor::listen, acceptor.get()));
}

TcpServer::~TcpServer() {
}

} // namespace net
} // namespace muduo
