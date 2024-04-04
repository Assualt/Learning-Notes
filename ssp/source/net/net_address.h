//
// Created by 侯鑫 on 2024/1/2.
//

#ifndef SSP_TOOLS_NET_ADDRESS_H
#define SSP_TOOLS_NET_ADDRESS_H

#include <string>
#include <cstdint>
#include <sys/un.h>
#include <netinet/in.h>

namespace ssp::net {
class InetAddress {
public:
    InetAddress() = default;
    ~InetAddress() = default;

    explicit InetAddress(const std::string &ip, uint16_t port);

    explicit InetAddress(uint16_t port, bool loopBackOnly = false, bool ipv6 = false);

    explicit InetAddress(const sockaddr_in6 &addr);

    explicit InetAddress(const sockaddr_in &addr);

    [[nodiscard]] sa_family_t Family() const { return uAddr_.sin_family; }

    [[nodiscard]] const struct sockaddr *GetSockAddr() const;

    void SetSockAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }

    [[nodiscard]] std::string ToIpPort() const;

    [[nodiscard]] std::string Host() const { return host_; }

    void SetHost(const std::string &host) { host_ = host; }

private:
    union {
        struct sockaddr_in  uAddr_{};
        struct sockaddr_in6 addr6_;
    };

    std::string host_;
};

}

#endif //SSP_TOOLS_NET_ADDRESS_H
