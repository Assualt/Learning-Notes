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

template<typename T>
class SocketImpl {
public:
    static SocketImpl CreateSocket(int32_t fd, void *args)
    {
        SocketImpl ins(fd, args);
        return ins;
    }

    void SetKeepAlive(bool on)
    {
        int opt = on ? 1 : 0;
        setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));
    }

    void SetReuseAddr(bool on)
    {
        int opt = on ? 1 : 0;
        ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
    }

    void SetReusePort(bool on)
    {
#ifdef SO_REUSEPORT
        int opt = on ? 1 : 0;
        int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
        if (ret < 0 && on) {
            logger.Error("set SO_REUSE PORT failed.");
        }
#endif
    }

private:
    explicit SocketImpl(int32_t fd, void *args = nullptr)
    {
        fd_ = fd;
        val.Init(fd, args);
    }

    ~SocketImpl()
    {
        Close();
    }

    void Close()
    {
        val.Close();
    }

private:
    T val;
    int32_t fd_ { -1 };
};


using Socket = SocketImpl<NormalSocket>;
using SslSocket = SocketImpl<SocketSsl>;

}


#endif //SSP_TOOLS_SOCKET_H
