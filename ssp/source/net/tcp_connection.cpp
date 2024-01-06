//
// Created by 侯鑫 on 2024/1/7.
//

#include "tcp_connection.h"

#include <utility>
#include "socket.h"

using namespace ssp::net;
TcpConnection::TcpConnection(EventLoop *loop, std::string name, int sockFd, const InetAddress &locAddr,
                       const InetAddress &peerAddr, void *ssl)

    : socket_(new Socket(sockFd, ssl))
    , channel_(new Channel(loop, socket_.get()))
    , loop_(loop)
    , connName_(std::move(name))
    , localAddr_(locAddr)
    , peerAddr_(peerAddr)
{
    channel_->SetChannelCallback(
        [this](auto t) { HandleRead(t);},
        [this](auto t) { HandleWrite(t);},
        [this](auto t) { HandleError(t);},
        [this](auto t) { HandleClose(t);}
    );
}

TcpConnection::~TcpConnection()
{
    socket_->ShutdownWrite();
}

void TcpConnection::HandleRead(const ssp::base::TimeStamp &stamp)
{
    std::stringbuf buffer;
    auto count = socket_->Read(buffer);
    if (count <= 0) { // read error
        logger.Info("read error");
        HandleClose(TimeStamp::Now());
    } else {
        logger.Info("handle read. data count:%d", count);
        if (msgCb_) {
            msgCb_(shared_from_this(), buffer, stamp);
        }
    }
}

void TcpConnection::HandleWrite(const ssp::base::TimeStamp &)
{
    logger.Info("handle write");
}

void TcpConnection::HandleError(const ssp::base::TimeStamp &)
{
    logger.Info("handle error");
}

void TcpConnection::HandleReadTimeout(const ssp::base::TimeStamp &)
{
    logger.Info("handle readtimeout");
}

void TcpConnection::HandleClose(const ssp::base::TimeStamp &)
{
    Destory();
}

void TcpConnection::Established()
{
    channel_->EnableRead();
}

void TcpConnection::SetMessageCallback(MessageCallback cb)
{
    msgCb_ = std::move(cb);
}

void TcpConnection::SetCloseCallback(CloseCallback cb)
{
    closeCb_ = std::move(cb);
}

void TcpConnection::Destory()
{
    if (closeCb_) {
        closeCb_();
    }
    channel_->Disabled();
    channel_->Remove();
    loop_->Quit();
}

void TcpConnection::Close()
{
    Destory();
}

int32_t TcpConnection::Send(const void *buffer, int32_t length)
{
    if (buffer == nullptr || length <= 0) {
        return 0;
    }

    return socket_->Write(buffer, length);
}