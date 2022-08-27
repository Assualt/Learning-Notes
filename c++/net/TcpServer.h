#pragma once
#include "Callback.h"
#include "base/Timestamp.h"
#include "base/nonecopyable.h"
#include "net/InetAddress.h"
#include <functional>
#include <map>
#include <memory>
using namespace muduo::base;
namespace muduo {
namespace net {
class EventLoop;
class TcpConnection;
class TcpServer;
class Buffer;
class Acceptor;
class EventLoopThreadPool;
enum AddressOption { REUSE_PORT, NO_REUSE_PORT };

class TcpServer : nonecopyable {
public:
public:
    TcpServer(EventLoop *loop, const InetAddress &addr, const std::string &serverName, bool useSSL = false,
              AddressOption option = REUSE_PORT);
    ~TcpServer();

public:
    std::string IpPort();

    const std::string &Name() const;

    EventLoop *getLoop() const;

    void SetThreadNum(int threadNum);

    void SetThreadInitCallback(const ThreadInitCallback &cb);

    void SetConnectionCallback(const ConnectionCallback &cb);

    void SetMessageCallback(const MessageCallback &cb);

    void Start();

    void Stop();

    void NewConnection(int sockFd, const InetAddress &peerAddress, void *);

    void RemoveConnection(const TcpConnectionPtr &conn);

    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

private:
    int                                     m_nThreadNum;
    std::string                             m_strServerName;
    std::map<std::string, TcpConnectionPtr> m_connectionMap;
    std::unique_ptr<Acceptor>               acceptor; // avoid revealing Acceptor
    ThreadInitCallback                      threadInitCallback;
    ConnectionCallback                      connectionCallback;
    MessageCallback                         messageCallback;
    EventLoop                              *m_pLoop{nullptr};
    std::shared_ptr<EventLoopThreadPool>    m_threadPool;
    int                                     m_nNextConnId;
    InetAddress                             m_address;
};

} // namespace net
} // namespace muduo
