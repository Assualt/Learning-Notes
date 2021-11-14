#include "Acceptor.h"
#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/SocketsOP.h"
#include <stdio.h>
using namespace muduo::base;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : m_pLoop(loop)
    , m_nAcceptSocket(sockets::createNonblockingOrDie(listenAddr.family()))
    , m_cAcceptChannel(loop, m_nAcceptSocket.fd())
    , m_bListening(false) {
    m_nAcceptSocket.setReuseAddr(true);
    m_nAcceptSocket.setReusePort(reuseport);
    m_nAcceptSocket.bindAddress(listenAddr);
    m_cAcceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    // m_cAcceptChannel.disableAll();
    // m_cAcceptChannel.remove();
    ::close(m_nIdleFd);
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
    InetAddress peerAddr;
    // FIXME loop until no more
    int connfd = m_nAcceptSocket.accept(&peerAddr);
    if (connfd >= 0) {
        // string hostport = peerAddr.toIpPort();
        logger.info("accept connected fd:%d", connfd);
        if (newConnectionCallback) {
            newConnectionCallback(connfd, peerAddr);
        } else {
            sockets::close(connfd);
            logger.info("no handle new connection callback. close it instead");
        }
    } else {
        logger.info("in Acceptor::handleRead");
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        // if (errno == EMFILE) {
        //     ::close(m_nIdleFd);
        //     m_nIdleFd = ::accept(m_cAcceptChannel.fd(), NULL, NULL);
        //     ::close(m_nIdleFd);
        //     m_nIdleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        // }
    }
}
