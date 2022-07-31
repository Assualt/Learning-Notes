#pragma once
#include "HttpUrl.h"
#include "net/Buffer.h"
#include "net/Socket.h"
#include "net/TcpClient.h"
#include <string>

enum class HTTP_STATUS : uint32_t { OK, FAILED };

class HttpConnection : muduo::net::TcpClient {
public:
    bool connect(const std::string &url);

    void setTimeOut(int seconds);

    int32_t send(const muduo::net::Buffer &reqBuf);

    int32_t recv(muduo::net::Buffer &respBuf);

protected:
    bool connect(const HttpUrl &url);

private:
    int                                    timedOut_{10};
    std::string                            connectUrl_;
    bool                                   useSsl_{false};
    std::unique_ptr<muduo::net::TcpClient> client_{nullptr};
};