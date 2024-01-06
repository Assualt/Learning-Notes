//
// Created by 侯鑫 on 2024/1/3.
//

#include "acceptor.h"

#include <utility>

using namespace ssp::net;

namespace {
constexpr auto g_defaultTimeout = 3s;
}

Acceptor::Acceptor(EventLoop *loop, const ssp::net::InetAddress &address, bool reUsePort)
    : sock_(std::make_unique<Socket>())
    , acceptChannel_(Channel(loop, sock_.get()))
{
    sock_->SetReuseAddr(reUsePort);
    sock_->SetReadTimeout(g_defaultTimeout);
    sock_->SetWriteTimeout(g_defaultTimeout);
    sock_->BindAddress(address);

    acceptChannel_.SetChannelCallback([this](auto stamp) { this->HandleRead(stamp); }, nullptr, nullptr, nullptr);
}

bool Acceptor::SwitchToSSL()
{
    return sock_->SwitchToSSL(false, "");
}

void Acceptor::HandleRead(const ssp::base::TimeStamp &stamp)
{
    InetAddress peerAddress;
    auto [sockFd, sslArgs] = sock_->Accept(peerAddress);
    logger.Info("accept connection from:[%s %d]", peerAddress.ToIpPort(), sockFd);

    if (newConnFn_ != nullptr) {
        newConnFn_(sockFd, peerAddress, sslArgs);
    }
}

void Acceptor::SetConnectionCallback(NewConnectionCallback fn)
{
    newConnFn_ = std::move(fn);
}

void Acceptor::Listen()
{
    sock_->Listen();
    acceptChannel_.EnableRead();
}