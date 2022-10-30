#pragma once
#include "Buffer.h"
#include "InetAddress.h"
#include "Socket.h"
#include <optional>

namespace muduo::net {

class TcpClient {
public:
    explicit TcpClient(bool useSsl = false);

    void setTimeOut(int connectTimeout, int sendTimeOut, int recvTimeout);

    bool connect(const std::string &host, uint16_t port, bool needSwitchSSL = false);

    bool connect(const InetAddress &address);

    int32_t sendBuf(const char *buf, uint32_t size);

    int32_t sendRequest(const Buffer &buffer);

    int32_t recvResponse(Buffer &recvBuf);

    void close();

    virtual void showTlsInfo();

    ~TcpClient();

private:
    uint32_t switchToSSLConnect(const InetAddress &address);

private:
    std::unique_ptr<Socket> socket_{nullptr};
    int                     fd_{0};
    int                     connectTimeOut_{0};
    int                     sendTimeOut_{0};
    int                     readTimeOut_{0};
    bool                    useSsl_{false};
};

} // namespace muduo::net