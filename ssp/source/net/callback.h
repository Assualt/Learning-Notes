//
// Created by 侯鑫 on 2024/1/7.
//

#ifndef SSP_TOOLS_CALLBACK_H
#define SSP_TOOLS_CALLBACK_H

#include <functional>
#include "base/timestamp.h"
#include "net/net_address.h"

using namespace ssp::base;
using namespace ssp::net;

namespace ssp::net {
using EventCallback = std::function<void(TimeStamp)>;

class TcpConnection;

struct NetCallback {
    EventCallback writeFn_;
    EventCallback closeFn_;
    EventCallback errFn_;
    EventCallback readFn_;
    EventCallback readTimeoutFn_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using NewConnectionCallback = std::function<void(int32_t peerFd, const ssp::net::InetAddress, void *arg)>;
using OnConnectionCallback = std::function<bool(TcpConnectionPtr conn)>;
using MessageCallback = std::function<void(TcpConnectionPtr conn, std::stringbuf &, const TimeStamp&)>;
using CloseCallback = std::function<void()>;
}
#endif //SSP_TOOLS_CALLBACK_H
