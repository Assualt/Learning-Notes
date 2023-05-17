#include "Acceptor.h"
#include "base/Logging.h"
#include "base/System.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketsOp.h"
#include <fcntl.h>

using namespace muduo::base;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddress, bool reUsePort)
    : m_pLoop(loop)
    , m_nAcceptSocket(sockets::createNonblockingOrDie(listenAddress.family()))
    , m_cAcceptChannel(loop, m_nAcceptSocket.fd())
    , m_bListening(false) {
    m_nAcceptSocket.setReuseAddr(true);
    m_nAcceptSocket.setReusePort(reUsePort);
    m_nAcceptSocket.bindAddress(listenAddress);
    m_cAcceptChannel.setReadCallback([this](auto) { handleRead(); });
}

Acceptor::~Acceptor() { closeSocket(); }

void Acceptor::InitSslPemKey(const std::string &certPath, const std::string &pemPath) {
    if (!System::Access(certPath, F_OK) || !System::Access(pemPath, F_OK)) {
        logger.warning("cert:%s or pem:%s is not exist pwd:%s", certPath, pemPath, System::CurrentPwd());
        return;
    }
    bool result = m_nAcceptSocket.initSSLServer(certPath, pemPath);
    LOG_SYSTEM.info("cert:%s and pem:%s init ssl socket, ret:%b", certPath, pemPath, result);
}

void Acceptor::closeSocket() {
    m_cAcceptChannel.disableAll();
    m_cAcceptChannel.remove();
    if (m_nIdleFd != -1) {
        ::close(m_nIdleFd);
    }
    m_nIdleFd = -1;
}

void Acceptor::listen() {
    m_pLoop->assertLoopThread();
    m_bListening = true;
    m_nAcceptSocket.listen();
    logger.info("begin to listen at fd:%d", m_nAcceptSocket.fd());
    m_cAcceptChannel.enableReading();
}

void Acceptor::handleRead() {
    m_pLoop->assertLoopThread();
    InetAddress peerAddress;
    // FIXME loop until no more

    auto [ connFd, ctx ] = m_nAcceptSocket.accept(&peerAddress);
    if (connFd >= 0) {
        string hostPort = peerAddress.toIpPort();
        logger.info("accept connected fd:%d, host:%s", connFd, hostPort);
        if (newConnectionCallback) {
            newConnectionCallback(connFd, peerAddress, ctx);
        } else {
            sockets::close(connFd);
            logger.info("no handle new connection callback. close it instead");
        }
    } else {
        logger.info("error in Acceptor::handleRead");
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.

        if (errno == EMFILE) {
            ::close(m_nIdleFd);
            m_nIdleFd = ::accept(m_cAcceptChannel.fd(), NULL, NULL);
            ::close(m_nIdleFd);
            m_nIdleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
