//
// Created by 侯鑫 on 2024/1/8.
//

#ifndef SSP_TOOLS_TCP_CLIENT_H
#define SSP_TOOLS_TCP_CLIENT_H

#include <cstdint>
#include <string>
#include "net/net_address.h"
#include "net/socket.h"
#include "net/buffer.h"

namespace ssp::net {
class TcpClient {
public:
    explicit TcpClient(bool useSsl = false);

    ~TcpClient();

public:
    void SetTimeOut(int32_t connTimeout, int32_t sendTimeout, int32_t rcvTimeout);

    bool Connect(const std::string& host, uint16_t port, bool switchSsl = false, bool verbose = false);

    bool Connect(const InetAddress &address, bool verbose = false);

    int32_t Send(const void *buffer, uint32_t length);

    int32_t Read(std::stringbuf &rcvBuf);

    int32_t Read(Buffer &buffer);

    void Close();

private:
    std::unique_ptr<Socket> sock_{nullptr};
    uint32_t connTimeout_{10};
    bool useSsl_{false};

};
}

#endif //SSP_TOOLS_TCP_CLIENT_H
