#pragma once
#include "base/nonecopyable.h"
#include "net/Channel.h"
#include "net/Socket.h"
#include <functional>
namespace muduo {
namespace net {

class EventLoop;
class InetAddress;

class Acceptor : base::nonecopyable {
public:
    typedef std::function<void(int, const InetAddress &)> NewConnectionCallback;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb) {
        newConnectionCallback = cb;
    }

    void listen();

    bool listening() const {
        return m_bListening;
    }

    // Deprecated, use the correct spelling one above.
    // Leave the wrong spelling here in case one needs to grep it for error messages.
    // bool listenning() const { return listening(); }

private:
    void handleRead();

    EventLoop *           m_pLoop;
    Socket                m_nAcceptSocket;
    Channel               m_cAcceptChannel;
    NewConnectionCallback newConnectionCallback;
    bool                  m_bListening;
    int                   m_nIdleFd;
};
} // namespace net
} // namespace muduo
