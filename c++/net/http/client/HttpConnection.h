#pragma once
#include "HttpUrl.h"
#include "net/Socket.h"
#include "net/Buffer.h"
#include <string>

enum class HTTP_STATUS : uint32_t {
    OK,
    FAILED
};

class HttpConnection {
public:
    bool connect(const std::string &url);

    void setTimeOut(int seconds);

    int32_t send(const muduo::net::Buffer &reqBuf);

    int32_t recv(muduo::net::Buffer &respBuf);

protected:
    bool connect(const HttpUrl &url);

    bool connectWithSSL();

private:
    int                                 timedOut_{10};
    std::string                         connectUrl_;
    bool                                useSsl_{false};
    std::unique_ptr<muduo::net::Socket> socket_{nullptr};
};