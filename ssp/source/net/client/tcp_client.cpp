//
// Created by 侯鑫 on 2024/1/8.
//

#include "tcp_client.h"
#include "base/log.h"
#include "base/system.h"
#include "net/endian.h"
#include "netdb.h"

using namespace ssp::base;
using namespace ssp::net;

TcpClient::TcpClient(bool useSsl)
    : sock_(std::make_unique<Socket>())
    , useSsl_(useSsl)
{
    if (useSsl) {
        sock_->SwitchToSSL(true, "");
    }
}

TcpClient::~TcpClient()
{
    Close();
}

void TcpClient::Close()
{
    sock_->Close();
}

void TcpClient::SetTimeOut(int32_t connTimeout, int32_t sendTimeout, int32_t rcvTimeout)
{
    sock_->SetReadTimeout(std::chrono::seconds(rcvTimeout));
    sock_->SetWriteTimeout(std::chrono::seconds(sendTimeout));
    connTimeout_ = connTimeout;
}

bool TcpClient::Connect(const ssp::net::InetAddress &address)
{
    return sock_->Connect(address, useSsl_, std::chrono::seconds(connTimeout_));
}

bool TcpClient::Connect(const std::string &host, uint16_t port, bool switchSsl)
{
    logger.Info("begin to connect %s:%d ...", host, port);
    auto hosts = gethostbyname(host.c_str());
    if (hosts == nullptr) {
        logger.Error("can't get host [%s:%d] ip yet.", host, port);
        return false;
    }

    for (int32_t idx = 0; hosts->h_addr_list[idx]; ++idx) {
        auto addr = InetAddress(sockaddr_in{
            .sin_family = static_cast<sa_family_t>(hosts->h_addrtype),
            .sin_port = sockets::HostToNetwork16(port),
            .sin_addr = *(struct in_addr *)hosts->h_addr_list[ idx ]
        });

        addr.SetHost(host);
        auto ret = Connect(addr);
        if (!ret) {
            logger.Info("connect with %s:%d failed error:%d", host, port, System::GetErrMsg(errno));
            sock_->Close();
            continue;
        }

        return true;
    }

    logger.Info("can't connect the url. %s:%d ", host, port);
    return false;
}

int32_t TcpClient::Send(const void *buffer, uint32_t length)
{
    if (buffer == nullptr || length == 0) {
        return -1;
    }

    return sock_->Write(buffer, length);
}

int32_t TcpClient::Read(std::stringbuf &rcvBuf)
{
    return sock_->Read(rcvBuf);
}