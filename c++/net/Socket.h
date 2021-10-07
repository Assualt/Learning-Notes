#pragma once
#include "base/nonecopyable.h"
#include <iomanip>
#include <iostream>
#include <netinet/tcp.h>
#include <sstream>

using muduo::base::nonecopyable;

namespace muduo {
namespace net {
class InetAddress;

class Socket : nonecopyable {
public:
    explicit Socket(int sockfd);
    ~Socket();

public:
    int  fd();
    bool getTcpInfo(struct tcp_info *) const;
    bool getTcpInfoString(std::string &infostring) const;

    void bindAddress(const InetAddress &addr);
    void listen();

    int accept(InetAddress &RemoteAddress);

    ///
    /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    ///
    void setTcpNoDelay(bool on);

    ///
    /// Enable/disable SO_REUSEADDR
    ///
    void setReuseAddr(bool on);

    ///
    /// Enable/disable SO_REUSEPORT
    ///
    void setReusePort(bool on);

    ///
    /// Enable/disable SO_KEEPALIVE
    ///
    void setKeepAlive(bool on);

private:
    int sockfd_{-1};
};
} // namespace net
} // namespace muduo