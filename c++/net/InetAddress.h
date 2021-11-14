#pragma once
#include <netinet/in.h>
namespace muduo {
namespace net {

class InetAddress {

public:
    InetAddress() = default;
    explicit InetAddress(uint16_t port, bool loopbackOnly = false, bool ipv6 = false);
    sa_family_t family() const {
        return m_uAddr.sin_family;
    }
    const struct sockaddr *getSockAddr() const;

    union {
        struct sockaddr_in  m_uAddr;
        struct sockaddr_in6 addr6_;
    };
};

} // namespace net
} // namespace muduo