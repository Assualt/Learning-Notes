//
// Created by 侯鑫 on 2024/1/3.
//

#include "acceptor.h"

using namespace ssp::net;

namespace {
constexpr auto g_defaultTimeout = 3s;
}

Acceptor::Acceptor(const ssp::net::InetAddress &address, bool reUsePort)
    :sock_(std::make_unique<Socket>())
{
    sock_->SetReuseAddr(reUsePort);
    sock_->SetReadTimeout(g_defaultTimeout);
    sock_->SetWriteTimeout(g_defaultTimeout);
    sock_->BindAddress(address);
}

bool Acceptor::SwitchToSSL()
{
    return sock_->SwitchToSSL(false, "");
}

