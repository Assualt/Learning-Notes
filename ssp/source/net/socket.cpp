//
// Created by 侯鑫 on 2024/1/1.
//

#include "socket.h"
#include "socket_ops.h"

using namespace ssp::net;

Socket::Socket()
{
    fd_ = sockets::CreateSocket();
    Init(nullptr);
}

Socket Socket::CreateSocket(void *args)
{
    return Socket{sockets::CreateSocket(), args};
}

void Socket::SetKeepAlive(bool on) const
{
    int opt = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));
}

void Socket::SetReuseAddr(bool on) const
{
    int opt = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
}

void Socket::SetReusePort(bool on) const
{
#ifdef SO_REUSEPORT
    int opt = on ? 1 : 0;
    int ret = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    if (ret < 0 && on) {
        logger.Error("set SO_REUSE PORT failed.");
    }
#endif
}

void Socket::SetWriteTimeout(std::chrono::seconds timeout) const
{
    struct timeval val { .tv_sec = timeout.count() };
    auto ret = setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, (const void *)&val, sizeof(val));
    if (ret == -1) {
        logger.Warning("set write timeout failed.");
    }
}

void Socket::SetReadTimeout(std::chrono::seconds timeout) const
{
    struct timeval val { .tv_sec = timeout.count() };
    auto ret = setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, (const void *)&val, sizeof(val));
    if (ret == -1) {
        logger.Warning("set read timeout failed.");
    }
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

bool Socket::SwitchToSSL(bool isClient, const std::string &host)
{
    if (normalSocket_ == nullptr) {
        return false;
    }

    return normalSocket_->SwitchSSL(isClient, host), true;
}


bool Socket::Connect(const InetAddress &addr, bool useSsl, std::chrono::seconds timeout)
{
    return normalSocket_->Connect(addr, useSsl, timeout);
}

void Socket::BindAddress(const InetAddress &address) const
{
    sockets::BindOrDie(fd_, address.GetSockAddr());
}
