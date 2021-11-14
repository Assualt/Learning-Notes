#pragma once
#include "base/Timestamp.h"
#include "base/nonecopyable.h"
#include <functional>
#include <map>
#include <memory>
using namespace muduo::base;
namespace muduo {
namespace net {
class EventLoop;
class InetAddress;
class TcpConnection;
class TcpServer;
class Buffer;
class Acceptor;
enum AddressOption { REUSE_PORT, NO_REUSE_PORT };

class TcpServer : nonecopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    using ConnectionCallback = std::function<void(const TcpConnection &)>;
    using MessageCallback    = std::function<void(const TcpConnection &, Buffer *, Timestamp)>;

public:
    TcpServer(EventLoop *loop, const InetAddress &addr, const std::string &serverName, AddressOption option = REUSE_PORT);
    ~TcpServer();

public:
    const std::string &ipPort() const;
    const std::string &name() const;
    EventLoop *        getLoop() const;

    void setThreadNum(int threadNum);
    void setThreadInitCallback(const ThreadInitCallback &cb);
    void setConnectionCallback(const ConnectionCallback &cb);
    void setMessageCallback(const MessageCallback &cb);

    void start();

    void newConnection(int sockfd, const InetAddress &peerAddress);

private:
    int                                  m_nThreadNum;
    std::string                          m_strServerName;
    std::map<std::string, TcpConnection> connectionMap;
    std::unique_ptr<Acceptor>            acceptor; // avoid revealing Acceptor
    ThreadInitCallback                   threadInitCallback;
    ConnectionCallback                   connectionCallback;
    MessageCallback                      messageCallback;
    EventLoop *                          m_pLoop{nullptr};
};

} // namespace net
} // namespace muduo
