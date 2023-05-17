#include "Acceptor.h"
#include "Buffer.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketsOp.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "base/Format.h"
#include "base/Logging.h"
#include <chrono>
#include <functional>
#include <thread>
using namespace std;
using namespace muduo::net;
namespace {
void defaultConnectionCallback(const TcpConnectionPtr &conn) {
    logger.info("Connection local address:[%s] -> peer address:[%s] ", conn->localAddress().toIpPort(),
                conn->peerAddress().toIpPort());
}

void defaultMessageCallback(const TcpConnectionPtr &, Buffer *buf, Timestamp) {
    logger.info("message call back ....");
    buf->retrieveAll();
}
} // namespace
void TcpServer::SetThreadNum(int threadNum) { m_threadPool->SetThreadNum(threadNum); }

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr, const std::string &serverName, AddressOption)
    : m_pLoop(loop)
    , m_strServerName(serverName)
    , acceptor(new Acceptor(loop, addr, false))
    , connectionCallback(defaultConnectionCallback)
    , messageCallback(defaultMessageCallback)
    , m_threadPool(new EventLoopThreadPool(loop, serverName))
    , m_nNextConnId(1)
    , m_address(addr) {
    acceptor->setNewConnectionCallback(
        [this](auto &&arg1, auto &&arg2, auto &&arg3) { NewConnection(arg1, arg2, arg3); });
    TimeOutCheckThread();
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
    ++m_nNextConnId;
    std::string connName = FmtString("%-%#%").arg(m_strServerName).arg(locAddr.toIpPort()).arg(m_nNextConnId).str();
    logger.info("TcpServer::newConnection [%s] - new connection [%s] from [%s]", m_strServerName, connName,
                peerAddress.toIpPort());
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockFd, locAddr, peerAddress, arg));
    conn->setConnectionCallBack(connectionCallback);
    conn->setMessageCallBack(messageCallback);
    conn->setCloseCallBack([this](auto &&PH1) { RemoveConnection(std::forward<decltype(PH1)>(PH1)); });
    ioLoop->runInLoop([conn] { conn->connectEstablished(); });
    m_lock.Lock();
    m_connectionMap[ connName ] = conn;
    m_lock.UnLock();
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn) {
    m_pLoop->runInLoop([this, conn] { RemoveConnectionInLoop(conn); });
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn) {
    m_pLoop->assertLoopThread();
    logger.info("TcpServer::RemoveConnectionInLoop name:%s - connection Name:%s", m_strServerName, conn->name());
    m_lock.Lock();
    size_t n = m_connectionMap.erase(conn->name());
    m_lock.UnLock();
    EventLoop *loop = conn->getLoop();
    loop->queueInLoop([conn] { conn->connectDestroyed(); });
}

void TcpServer::SetConnectionCallback(const ConnectionCallback &callback) { connectionCallback = callback; }

void TcpServer::SetMessageCallback(const MessageCallback &callback) { messageCallback = callback; }

void TcpServer::SetThreadInitCallback(const ThreadInitCallback &callback) { threadInitCallback = callback; }

void TcpServer::Start() {
    m_threadPool->start(threadInitCallback);
    m_pLoop->runInLoop([capture0 = acceptor.get()] { capture0->listen(); });
    logger.info("All is ok........");
}

void TcpServer::Stop() { acceptor->closeSocket(); }

void TcpServer::SetMsgTimeoutCallback(uint32_t timeout, MsgTimeOutCallback cb) {
    m_nMsgTimeout      = timeout;
    msgTimeOutCallback = cb;
    logger.info("set msg timeout:%ds,and cb is not null:%b", timeout, (cb != nullptr));
}

void TcpServer::TimeOutCheckThread() {
    auto threadFunc = [this]() {
        while (!m_bNeedExit && (m_nMsgTimeout != 0)) {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            m_lock.Lock();
            auto                     now = Timestamp::now();
            std::vector<std::string> timeOutConnectVectors;
            for (auto &[ conName, connPtr ] : m_connectionMap) {
                if (connPtr == nullptr) {
                    continue;
                }

                if (connPtr->isMsgProcessing()) {
                    continue;
                }

                auto lastTimeStamp = connPtr->getLastMsgTimeStamp();
                if (lastTimeStamp.seconds() == 0) {
                    continue;
                }

                if (now.seconds() - lastTimeStamp.seconds() >= m_nMsgTimeout) {
                    if (msgTimeOutCallback) {
                        msgTimeOutCallback(connPtr);
                    }
                    timeOutConnectVectors.push_back(conName);
                    connPtr->shutdownInLoop();
                    logger.info("current conn is timeout %s, lastTime:%s", conName, lastTimeStamp.toFmtString());
                }
            }
            m_lock.UnLock();
            for (auto &item : timeOutConnectVectors) {
                RemoveConnection(m_connectionMap[ item ]);
            }
        }
    };

    m_checkThread = std::make_unique<Thread>(threadFunc, "LoopConnCheck");
    m_checkThread->Start();
}

TcpServer::~TcpServer() {
    m_bNeedExit = true;
    if (m_checkThread && m_checkThread->IsStarted()) {
        m_checkThread->Join();
    }
}
const std::string &TcpServer::Name() const { return m_strServerName; }

void TcpServer::InitSslConfig(const std::string &certPath, const std::string &pemPath) {
    acceptor->InitSslPemKey(certPath, pemPath);
}
