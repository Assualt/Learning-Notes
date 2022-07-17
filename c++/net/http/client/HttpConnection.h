#pragma once
#include "HttpUrl.h"
#include "net/Socket.h"
#include <string>
class HttpConnection {
public:
    bool connect(const std::string &url);

    void setTimeOut(int seconds);

protected:
    bool connect(const HttpUrl &url);

private:
    int                                 timedOut_{10};
    std::string                         connectUrl_;
    bool                                useSsl_{false};
    std::shared_ptr<muduo::net::Socket> socket_{nullptr};
};