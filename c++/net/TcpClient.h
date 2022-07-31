#pragma once
#include "Buffer.h"
#include "InetAddress.h"
#include "Socket.h"
#include "TcpConnection.h"

namespace muduo::net {

class TcpClient {
public:
    TcpClient(bool useSsl = false);

    void setTimeOut(int connectTimeout, int sendTimeOut, int recvTimeout);

    bool connect(const InetAddress &address);

    int32_t sendBuf(const char *buf, uint32_t size);

    int32_t sendRequest(const Buffer &buffer);

    int32_t recvResponse(Buffer &recvBuf);

    void close();

    ~TcpClient();

private:
    uint32_t switchToSSLConnect();

private:
    std::unique_ptr<Socket>        socket_{nullptr};
    std::unique_ptr<TcpConnection> conn_{nullptr};
    int                            fd_{0};
    int                            connectTimeOut_{0};
    int                            sendTimeOut_{0};
    int                            readTimeOut_{0};
    bool                           useSsl_{false};
};

} // namespace muduo::net