//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_ACCEPTOR_H
#define SSP_TOOLS_ACCEPTOR_H

#include "net/socket.h"

namespace ssp::net {
class Acceptor {
public:
    explicit Acceptor(const InetAddress &address, bool reUsePort = true);

    bool SwitchToSSL();

private:
    std::unique_ptr<Socket> sock_{nullptr};
};
}

#endif //SSP_TOOLS_ACCEPTOR_H
