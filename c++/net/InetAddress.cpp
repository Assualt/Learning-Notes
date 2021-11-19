#include "InetAddress.h"
#include "Endian.h"
#include "SocketsOp.h"
#include "base/Format.h"
#include <netinet/in.h>
#include <strings.h>
using namespace muduo::net;
using namespace muduo::base;

static const in_addr_t kInaddrAny      = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
    if (ipv6) {
        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip        = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr   = ip;
        addr6_.sin6_port   = sockets::hostToNetwork16(port);
    } else {
        bzero(&m_uAddr, sizeof m_uAddr);
        m_uAddr.sin_family      = AF_INET;
        in_addr_t ip            = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        m_uAddr.sin_addr.s_addr = sockets::hostToNetwork32(ip);
        m_uAddr.sin_port        = sockets::hostToNetwork16(port);
    }
}

InetAddress::InetAddress(const sockaddr_in &addr)
    : m_uAddr(addr) {
}

InetAddress::InetAddress(const sockaddr_in6 &addr)
    : addr6_(addr) {
}

std::string InetAddress::toIpPort() const {
    return FmtString("%:%").arg(inet_ntoa(m_uAddr.sin_addr)).arg(m_uAddr.sin_port).str();
}

const struct sockaddr *InetAddress::getSockAddr() const {
    return (const struct sockaddr *)(&m_uAddr);
}