#pragma once

#include "Buffer.h"
#include "Callback.h"
#include "InetAddress.h"
#include "base/Timestamp.h"
#include <any>
#include <bits/shared_ptr.h>
#include <iostream>
#include <memory>

namespace muduo {
namespace net {

class EventLoop;
class Socket;
class Channel;
enum class TcpState : uint8_t { DisConnected, Connecting, Connected, DisConnecting };

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop, const std::string &name, int sockFd, const InetAddress &locAddr, const InetAddress &peerAddr, void *ssl = nullptr);

    ~TcpConnection();

public:
    void handleRead(const Timestamp &tistamp);
    void handleWrite();
    void handleClose();
    void handleError();
    void connectEstablished();
    void connectDestory();
    void setConnectionCallBack(ConnectionCallback cb) {
        m_connCallBack = cb;
    }

    void setMessageCallBack(MessageCallback cb) {
        m_messCallBack = cb;
    }

    void setWriteCompleteCallBack(WriteCompleteCallback cb) {
        m_writecompCallBack = cb;
    }

    void setHighWaterCallBack(HighWaterMarkCallback cb) {
        m_hightwaterCallBack = cb;
    }

    void setCloseCallBack(CloseCallback cb) {
        m_closeCallBack = cb;
    }

    const InetAddress &localAddress() const {
        return m_locAddr;
    }

    const InetAddress &peerAddress() const {
        return m_peerAddr;
    }

    const std::string &name() const {
        return m_strName;
    }

    EventLoop *getLoop() {
        return m_pLoop;
    }

    bool isConnected() {
        return m_state == TcpState::Connected;
    }

    void send(const void *message, int len);

    void send(Buffer *buf);

    void shutdown();

    void shutdownInLoop();

protected:
    void sendInLoop(const void *data, size_t len);

protected:
    EventLoop               *m_pLoop;
    const InetAddress        m_locAddr;
    const InetAddress        m_peerAddr;
    TcpState                 m_state;
    std::string              m_strName;
    std::unique_ptr<Socket>  m_socket;
    std::unique_ptr<Channel> m_channel;

    // call backs
    ConnectionCallback    m_connCallBack;
    MessageCallback       m_messCallBack;
    HighWaterMarkCallback m_hightwaterCallBack;
    WriteCompleteCallback m_writecompCallBack;
    CloseCallback         m_closeCallBack;
    // Buffer
    Buffer m_input;
    Buffer m_output;
};

} // namespace net
} // namespace muduo