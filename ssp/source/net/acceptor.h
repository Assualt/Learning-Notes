//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_ACCEPTOR_H
#define SSP_TOOLS_ACCEPTOR_H

#include "net/socket.h"
#include "net/channel.h"

namespace ssp::net {

class EventLoop;
class Acceptor {
public:
    explicit Acceptor(EventLoop *loop, const InetAddress &address, bool reUsePort = true);

    bool SwitchToSSL();

    void Listen();

    void SetConnectionCallback(NewConnectionCallback fn);

private:
    void HandleRead(const TimeStamp &stamp);

private:
    std::unique_ptr<Socket> sock_{nullptr};
    Channel acceptChannel_;
    NewConnectionCallback newConnFn_{nullptr};
};
}

#endif //SSP_TOOLS_ACCEPTOR_H
