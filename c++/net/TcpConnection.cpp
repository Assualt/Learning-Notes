#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOp.h"
#include "base/Logging.h"
#include <functional>
using namespace std;
using namespace muduo::net;
#define HIGH_WARTE_MARK (64 * 1024 * 1024)

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockFd, const InetAddress &locAddr,
                             const InetAddress &peerAddr, void *ssl)
    : m_pLoop(loop)
    , m_locAddr(locAddr)
    , m_peerAddr(peerAddr)
    , m_strName(name)
    , m_state(TcpState::Connecting)
    , m_socket(new Socket(sockFd, ssl))
    , m_channel(new Channel(loop, sockFd)) {

    m_channel->setReadCallback([ this ](const auto &time) { return handleRead(time); });
    m_channel->setWriteCallback([ this ]() { return handleWrite(); });
    m_channel->setCloseCallback([ this ]() { return handleClose(); });
    m_channel->setErrorCallback([ this ]() { return handleError(); });
    m_channel->setReadTimeOutCallback([ this ]() { return shutdown(); });
    m_socket->setKeepAlive(false);
}

TcpConnection::~TcpConnection() { m_socket->shutdownWrite(); }

void TcpConnection::handleRead(const Timestamp &timeStamp) {
    m_pLoop->assertLoopThread();
    int     error   = 0;
    ssize_t readNum = m_socket->read(m_input);
    //    ssize_t readNum = m_input.readFd(m_channel->fd(), &error);
    if (readNum > 0) {
        m_isMsgProcessing  = true;
        m_lastMsgTimeStamp = timeStamp;
        m_messCallBack(shared_from_this(), &m_input, timeStamp);
        m_isMsgProcessing = false;
    } else if (readNum == 0) {
        handleClose();
    } else {
        errno = error;
        logger.warning("TcpConnection::HandleRead error. error:%d", errno);
        shutdownInLoop();
    }
}

void TcpConnection::handleWrite() {
    m_pLoop->assertLoopThread();
    if (m_channel->isWriting()) {
        ssize_t writeBytes = m_socket->write((void *)m_output.peek(), m_output.readableBytes());
        if (writeBytes) {
            m_output.retrieve(writeBytes);
            if (m_state == TcpState::DisConnecting) {
                shutdownInLoop();
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

void TcpConnection::handleError() { m_pLoop->assertLoopThread(); }

void TcpConnection::connectEstablished() {
    m_pLoop->assertLoopThread();
    m_channel->enableReading();
    m_state = TcpState::Connected;
    m_connCallBack(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    m_pLoop->assertLoopThread();
    if (m_state == TcpState::Connected) {
        m_channel->disableAll();
        m_state = TcpState::DisConnected;
        m_connCallBack(shared_from_this());
    }
    m_channel->remove();
}

void TcpConnection::send(const void *message, int len) {
    if (m_state != TcpState::Connected) {
        logger.error("can't send buf with wrong state. cur state:%d", static_cast<uint32_t>(m_state));
        return;
    }
    if (m_pLoop->isInLoopThread()) {
        sendInLoop(message, len);
    }
}

void TcpConnection::send(Buffer *buf) {
    if (m_state != TcpState::Connected) {
        logger.error("can't send buf with wrong state. cur state:%d", static_cast<uint32_t>(m_state));
        return;
    }

    if (m_pLoop->isInLoopThread()) {
        sendInLoop(buf->peek(), buf->readableBytes());
        buf->retrieveAll();
    }
}

void TcpConnection::shutdown() {
    if (m_state != TcpState::Connected) {
        logger.error("can't shutdown with wrong state. cur state:%d", static_cast<uint32_t>(m_state));
        return;
    }
    m_state = TcpState::DisConnecting;
    // FIXME: shared_from_this()?
    m_pLoop->runInLoop([ this ] { shutdownInLoop(); });
}

void TcpConnection::shutdownInLoop() {
    m_pLoop->assertLoopThread();
    if (!m_channel->isWriting()) {
        // we are not writing
        m_socket->shutdownWrite();
#ifdef USE_SSL
        m_socket->sslDisConnect();
#endif
    }
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    m_pLoop->assertLoopThread();
    ssize_t nWrote     = 0;
    size_t  remaining  = len;
    bool    faultError = false;
    if (m_state == TcpState::DisConnected) {
        logger.info("disconnected, give up writing");
        return;
    }
    // if nothing in output queue, try writing directly
    if (!m_channel->isWriting() && m_output.readableBytes() == 0) {
        nWrote = m_socket->write(const_cast<void *>(data), len);
        if (nWrote >= 0) {
            remaining = len - nWrote;
            if (remaining == 0 && m_writecompCallBack) {
                m_pLoop->queueInLoop(std::bind(m_writecompCallBack, shared_from_this()));
            }
        } else // nWrote < 0
        {
            nWrote = 0;
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
        m_output.append(static_cast<const char *>(data) + nWrote, remaining);
        if (!m_channel->isWriting()) {
            m_channel->enableWriting();
        }
    }
}