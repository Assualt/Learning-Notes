#pragma once
#include <iostream>
#include <memory>
#include <netinet/in.h>
namespace muduo {
namespace net {

class InetAddress {

public:
    InetAddress() = default;

    explicit InetAddress(uint16_t port, bool loopBackOnly = false, bool ipv6 = false);

    explicit InetAddress(const sockaddr_in6 &addr);

    explicit InetAddress(const sockaddr_in &addr);

    sa_family_t family() const { return m_uAddr.sin_family; }

    const struct sockaddr *getSockAddr() const;

    void setSockAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }

    std::string toIpPort() const;

    union {
        struct sockaddr_in  m_uAddr;
        struct sockaddr_in6 addr6_;
    };
};

} // namespace net
} // namespace muduo