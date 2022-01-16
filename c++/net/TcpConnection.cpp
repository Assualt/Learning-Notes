#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOp.h"
#include "TcpConnection.h"
#include "base/Logging.h"
#include <bits/shared_ptr.h>
#include <functional>
using namespace std;
using namespace muduo::net;
#define HIGH_WARTE_MARK (64 * 1024 * 1024)

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &locAddr, const InetAddress &peerAddr)
    : m_pLoop(loop)
    , m_locAddr(locAddr)
    , m_peerAddr(peerAddr)
    , m_strName(name)
    , m_state(TcpState::Connecting)
    , m_socket(new Socket(sockfd))
    , m_channel(new Channel(loop, sockfd)) {

    m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    m_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    m_channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    m_socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
}

void TcpConnection::handleRead(const Timestamp &tistamp) {
    m_pLoop->assertLoopThread();
    int     error;
    ssize_t readNum = m_input.readFd(m_channel->fd(), &error);
    if (readNum > 0) {
        m_messCallBack(shared_from_this(), &m_input, tistamp);
    } else if (readNum == 0) {
        handleClose();
    } else {
        errno = error;
        logger.warning("TcpConnection::HandleRead error. error:%d", error);
    }
}

void TcpConnection::handleWrite() {
    m_pLoop->assertLoopThread();
    if (m_channel->isWriting()) {
        ssize_t writeBytes = sockets::write(m_channel->fd(), m_output.peek(), m_output.readableBytes());
        if (writeBytes) {
            m_output.retrieve(writeBytes);
            if (m_state == TcpState::DisConnecting) {
                // shutdownInLoop();
            }
        } else {
            logger.warning("TcpConnection::HandleWrite failed");
        }
    } else {
        logger.warning("Connection fd=%d is down.", m_channel->fd());
    }
}

void TcpConnection::handleClose() {
    m_pLoop->assertLoopThread();
    logger.info("ip:%s change fd=%d state=%d", m_peerAddr.toIpPort(), m_channel->fd(), static_cast<int>(m_state));
    m_state = TcpState::DisConnecting;

    m_closeCallBack(shared_from_this());
}

void TcpConnection::handleError() {
    m_pLoop->assertLoopThread();
}

void TcpConnection::connectEstablished() {
    m_pLoop->assertLoopThread();
    m_channel->enableReading();
    m_state = TcpState::Connected;
    m_connCallBack(shared_from_this());
}

void TcpConnection::connectDestory() {
    m_pLoop->assertLoopThread();
    if (m_state == TcpState::Connected) {
        m_channel->disableAll();
        m_state = TcpState::DisConnected;
        m_connCallBack(shared_from_this());
    }
    m_channel->remove();
}

void TcpConnection::send(const void *message, int len) {
    if (m_state == TcpState::Connected) {
        if (m_pLoop->isInLoopThread()) {
            sendInLoop(message, len);
        } else {
            //
        }
    }
}

void TcpConnection::send(Buffer *buf) {
    if (m_state == TcpState::Connected) {
        if (m_pLoop->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        } else {
            //
        }
    }
}

void TcpConnection::shutdown() {
    if (m_state == TcpState::Connected) {
        m_state = TcpState::DisConnecting;
        // FIXME: shared_from_this()?
        m_pLoop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    m_pLoop->assertLoopThread();
    if (!m_channel->isWriting()) {
        // we are not writing
        m_socket->shutdownWrite();
    }
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    m_pLoop->assertLoopThread();
    ssize_t nwrote     = 0;
    size_t  remaining  = len;
    bool    faultError = false;
    if (m_state == TcpState::DisConnected) {
        logger.info("disconnected, give up writing");
        return;
    }
    // if no thing in output queue, try writing directly
    if (!m_channel->isWriting() && m_output.readableBytes() == 0) {
        nwrote = sockets::write(m_channel->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && m_writecompCallBack) {
                m_pLoop->queueInLoop(std::bind(m_writecompCallBack, shared_from_this()));
            }
        } else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                logger.info("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = m_output.readableBytes();
        if (oldLen + remaining >= HIGH_WARTE_MARK && oldLen < HIGH_WARTE_MARK && m_hightwaterCallBack) {
            m_pLoop->queueInLoop(std::bind(m_hightwaterCallBack, shared_from_this(), oldLen + remaining));
        }
        m_output.append(static_cast<const char *>(data) + nwrote, remaining);
        if (!m_channel->isWriting()) {
            m_channel->enableWriting();
        }
    }
}