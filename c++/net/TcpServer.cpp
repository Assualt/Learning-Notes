#include "TcpServer.h"
#include "net/Acceptor.h"
#include "net/Buffer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"
#include <functional>
using namespace std;
namespace muduo {
namespace net {

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr, const std::string &serverName, AddressOption option)
    : m_pLoop(loop)
    , m_strServerName(serverName)
    , acceptor(new Acceptor(loop, addr, false)) {
    acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddress) {
    EventLoop *ioLoop; // == thread->getNextLoop();
    m_pLoop->assertLoopThread();
    // 处理新链接
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
    m_pLoop->runInLoop(std::bind(&Acceptor::listen, acceptor.get()));
}

TcpServer::~TcpServer() {
}

} // namespace net
} // namespace muduo
