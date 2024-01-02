//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_SOCKET_H
#define SSP_TOOLS_SOCKET_H

#include <cstdint>
#include <sys/socket.h>
#include "base/log.h"
#include "socket_typedef.h"

namespace ssp::net {

class Socket {
public:
    static Socket CreateSocket(void *args);

    void SetKeepAlive(bool on);

    void SetReuseAddr(bool on);

    void SetReusePort(bool on);

    void Close();

private:
    Socket(int32_t fd, void *args);

    ~Socket();

    void Init(void *args);

private:
    int32_t fd_ { -1 };
    std::unique_ptr<NormalSocket> normalSocket_{nullptr};
};
}


#endif //SSP_TOOLS_SOCKET_H
