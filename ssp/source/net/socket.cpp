//
// Created by 侯鑫 on 2024/1/1.
//

#include "socket.h"
#include "socket_ops.h"

using namespace ssp::net;

Socket Socket::CreateSocket(void *args)
{
    return {sockets::CreateSocket(), args};
}

void Socket::SetKeepAlive(bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));
}

void Socket::SetReuseAddr(bool on)
{
    int opt = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
}

void Socket::SetReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int opt = on ? 1 : 0;
    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    if (ret < 0 && on) {
        logger.Error("set SO_REUSE PORT failed.");
    }
#endif
}

void Socket::Close()
{
    if (fd_ == -1) {
        return;
    }

    normalSocket_->Close();
    sockets::Close(fd_);
    fd_ = -1;
}

Socket::Socket(int32_t fd, void *args)
    :fd_(fd)
{
    Init(args);
}

Socket::~Socket()
{
    Close();
}

void Socket::Init(void *args)
{
    normalSocket_ = std::make_unique<NormalSocket>();
    normalSocket_->Init(fd_, args);
}
