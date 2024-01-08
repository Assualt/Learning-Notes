//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_SOCKET_H
#define SSP_TOOLS_SOCKET_H

#include <cstdint>
#include <chrono>
#include <sys/socket.h>
#include "base/log.h"
#include "socket_typedef.h"
#include "net_address.h"

using namespace std::chrono_literals;
namespace ssp::net {
class Socket {
public:
    Socket();

    explicit Socket(int32_t fd, void *args);

    static Socket CreateSocket(void *args);

    void SetKeepAlive(bool on) const;

    void SetReuseAddr(bool on) const;

    void SetReusePort(bool on) const;

    void SetWriteTimeout(std::chrono::seconds timeout) const;

    void SetReadTimeout(std::chrono::seconds timeout) const;

    void Close();

    bool Connect(const InetAddress &address, bool useSsl = false, std::chrono::seconds timeout = 3s);

    void BindAddress(const InetAddress &address) const;

    ~Socket();

    bool SwitchToSSL(bool isClient, const std::string &host);

    int32_t Fd() const { return normalSocket_->Fd(); };

    void Listen() const;

    std::pair<int32_t, void *> Accept(InetAddress &address);

    int32_t Read(std::stringbuf& buffer);

    int32_t Write(const void *buffer, uint32_t length);

    void ShutdownWrite();

private:
    void Init(void *args);

private:
    int32_t fd_ { -1 };
    std::unique_ptr<NormalSocket> normalSocket_{nullptr};
};
}


#endif //SSP_TOOLS_SOCKET_H
