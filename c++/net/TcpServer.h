#pragma once
#include "Callback.h"
#include "base/Mutex.h"
#include "base/Thread.h"
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

    [[nodiscard]] const std::string &Name() const;

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

    void SetMsgTimeoutCallback(uint32_t timeout, MsgTimeOutCallback);

private:
    // 检查connect是否有数据通信，如果长时间没有数据，需要主动断开链接
    void TimeOutCheckThread();

private:
    uint32_t                                m_nMsgTimeout{5};
    std::string                             m_strServerName;
    std::map<std::string, TcpConnectionPtr> m_connectionMap;
    std::unique_ptr<Acceptor>               acceptor; // avoid revealing Acceptor
    ThreadInitCallback                      threadInitCallback;
    ConnectionCallback                      connectionCallback;
    MessageCallback                         messageCallback;
    MsgTimeOutCallback                      msgTimeOutCallback{nullptr}; // 消息间隔超时处理函数
    EventLoop                              *m_pLoop{nullptr};
    std::shared_ptr<EventLoopThreadPool>    m_threadPool;
    int                                     m_nNextConnId;
    InetAddress                             m_address;
    std::unique_ptr<Thread>                 m_checkThread;
    MutexLock                               m_lock;
    bool                                    m_bNeedExit{false};
};

} // namespace net
} // namespace muduo
