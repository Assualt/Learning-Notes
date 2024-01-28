//
// Created by 侯鑫 on 2024/1/1.
//

#include "socket.h"
#include "socket_ops.h"

using namespace ssp::net;

namespace {
constexpr uint32_t g_maxBufferLine = 8192;
}

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


bool Socket::Connect(const InetAddress &addr, bool verbose, bool useSsl, std::chrono::seconds timeout)
{
    return normalSocket_->Connect(addr, verbose, useSsl, timeout);
}

void Socket::BindAddress(const InetAddress &address) const
{
    sockets::BindOrDie(fd_, address.GetSockAddr());
}

void Socket::Listen() const
{
    sockets::ListenOrDie(fd_);
}

std::pair<int32_t, void *> Socket::Accept(InetAddress &address)
{
    struct sockaddr_in6 addr{};
    bzero(&addr, sizeof addr);
    auto fd = sockets::Accept(fd_, &addr);
    if (fd < 0) {
        return {-1, nullptr};
    }

    address.SetSockAddrInet6(addr);

    if (!normalSocket_->IsSupportSSL()) {
        return {fd, nullptr};
    }

#if SUPPORT_OPENSSL
    auto val = normalSocket_->AcceptWithSSL(fd);
    return {fd, val};
#else
    return {fd, nullptr};
#endif
}

int32_t Socket::ReadImpl(const BufferFillFunc& func)
{
    if (func == nullptr || fd_ == -1) {
        return -1;
    }

    auto temp = std::make_unique<uint8_t[]>(g_maxBufferLine);
    if (temp == nullptr) {
        return -1;
    }

    int32_t total = 0;
    int32_t perRead = 0;

    while (true) {
        memset(temp.get(), 0, g_maxBufferLine);
        perRead = normalSocket_->Read(temp.get(), g_maxBufferLine);

        if (perRead < 0) {
            break;
        }

        if (!func(reinterpret_cast<const char *>(temp.get()), perRead)) {
             break; // 写入failed
        }
        total += perRead;

        if (perRead < g_maxBufferLine) {
            break;
        }
    }

    return total;
}

int32_t Socket::Read(ssp::net::Buffer &buffer)
{
    return ReadImpl([&buffer](const char *data, uint32_t len) {
        buffer.append(data, len);
        return true;
    });
}

int32_t Socket::Read(std::stringbuf &buffer)
{
    return ReadImpl([&buffer](const char *data, uint32_t len) {
        buffer.sputn(data, len);
        return true;
    });
}

int32_t Socket::Write(const void *buffer, uint32_t length)
{
    if (buffer == nullptr || length <= 0) {
        return -1;
    }

    if (fd_ == -1) {
        return -1;
    }

    return normalSocket_->Write(buffer, length);
}

void Socket::ShutdownWrite() const
{
    sockets::ShutdownWrite(fd_);
}