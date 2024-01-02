//
// Created by 侯鑫 on 2024/1/2.
//

#include "net_address.h"
#include "endian.h"
#include "socket_ops.h"
#include "base/format.h"

using namespace ssp::base;
using namespace ssp::net;

InetAddress::InetAddress(const std::string &ip, uint16_t port) 
{
    bzero(&uAddr_, sizeof uAddr_);
    uAddr_.sin_family      = AF_INET;
    uAddr_.sin_addr.s_addr = sockets::HostToNetwork32(inet_addr(ip.c_str()));
    uAddr_.sin_port        = sockets::HostToNetwork16(port);
}

InetAddress::InetAddress(uint16_t port, bool loopBackOnly, bool ipv6)
{
    if (ipv6) {
        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip        = loopBackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr   = ip;
        addr6_.sin6_port   = sockets::HostToNetwork16(port);
        return;
    }

    bzero(&uAddr_, sizeof uAddr_);
    uAddr_.sin_family      = AF_INET;
    in_addr_t ip            = loopBackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    uAddr_.sin_addr.s_addr = sockets::HostToNetwork32(ip);
    uAddr_.sin_port        = sockets::HostToNetwork16(port);
}

InetAddress::InetAddress(const sockaddr_in &addr)
    : uAddr_(addr)
{
}

InetAddress::InetAddress(const sockaddr_in6 &addr)
    : addr6_(addr) 
{}

std::string InetAddress::ToIpPort() const
{
    return FmtString("%:%").arg(inet_ntoa(uAddr_.sin_addr)).arg(sockets::NetworkToHost16(uAddr_.sin_port)).str();
}

const struct sockaddr *InetAddress::GetSockAddr() const
{
    return (const struct sockaddr *)(&uAddr_);
}

