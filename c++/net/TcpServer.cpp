#include "TcpServer.h"
#include "Acceptor.h"
#include "Buffer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketsOp.h"
#include "TcpConnection.h"
#include "base/Format.h"
#include "base/Logging.h"
#include <functional>
using namespace std;
namespace muduo {
namespace net {
void defaultConnectionCallback(const TcpConnectionPtr &conn) {
    logger.info("Connection local address:[%s] -> peer address:[%s] ", conn->localAddress().toIpPort(),
                conn->peerAddress().toIpPort());
}

void defaultMessageCallback(const TcpConnectionPtr &, Buffer *buf, Timestamp) {
    logger.info("message call back ....");
    buf->retrieveAll();
}

void TcpServer::SetThreadNum(int threadNum) { m_threadPool->SetThreadNum(threadNum); }

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr, const std::string &serverName, bool useSSL,
                     AddressOption)
    : m_pLoop(loop)
    , m_strServerName(serverName)
    , acceptor(new Acceptor(loop, addr, false, useSSL))
    , connectionCallback(defaultConnectionCallback)
    , messageCallback(defaultMessageCallback)
    , m_threadPool(new EventLoopThreadPool(loop, serverName))
    , m_nNexcConnId(1)
    , m_address(addr) {
    acceptor->setNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1,
                                                 std::placeholders::_2, std::placeholders::_3));
}

std::string TcpServer::IpPort() { return m_address.toIpPort(); }

void TcpServer::NewConnection(int sockFd, const InetAddress &peerAddress, void *arg) {
    m_pLoop->assertLoopThread();
    auto ioLoop = m_threadPool->getNextLoop();
    if (ioLoop == nullptr) {
        logger.alert("next eventLoop is empty. error");
        // need free arg;
#ifdef USE_SSL
        if (arg != nullptr) {
            SSL_shutdown(reinterpret_cast<SSL *>(arg));
        }
#endif
        return;
    }
    InetAddress locAddr(sockets::getLocalAddr(sockFd));
    ++m_nNexcConnId;
    std::string connName = FmtString("%-%#%").arg(m_strServerName).arg(locAddr.toIpPort()).arg(m_nNexcConnId).str();
    logger.info("TcpServer::newConnection [%s] - new connection [%s] from [%s]", m_strServerName, connName,
                peerAddress.toIpPort());
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockFd, locAddr, peerAddress, arg));
    conn->setConnectionCallBack(connectionCallback);
    conn->setMessageCallBack(messageCallback);
    conn->setCloseCallBack(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    m_connectionMap[ connName ] = conn;
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn) {
    m_pLoop->runInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn) {
    m_pLoop->assertLoopThread();
    logger.info("TcpServer::RemoveConnectionInLoop name:%s - connection Name:%s", m_strServerName, conn->name());
    size_t     n    = m_connectionMap.erase(conn->name());
    EventLoop *loop = conn->getLoop();
    loop->queueInLoop(std::bind(&TcpConnection::connectDestory, conn));
}

void TcpServer::SetConnectionCallback(const ConnectionCallback &callback) { connectionCallback = callback; }

void TcpServer::SetMessageCallback(const MessageCallback &callback) { messageCallback = callback; }

void TcpServer::SetThreadInitCallback(const ThreadInitCallback &callback) { threadInitCallback = callback; }

void TcpServer::Start() {
    m_threadPool->start(threadInitCallback);
    m_pLoop->runInLoop(std::bind(&Acceptor::listen, acceptor.get()));
    logger.info("All is ok........");
}

void TcpServer::Stop() { acceptor->closeSocket(); }

TcpServer::~TcpServer() {}

} // namespace net
} // namespace muduo
