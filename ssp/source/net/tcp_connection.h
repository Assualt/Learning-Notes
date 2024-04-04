//
// Created by 侯鑫 on 2024/1/7.
//

#ifndef SSP_TOOLS_TCP_CONNECTION_H
#define SSP_TOOLS_TCP_CONNECTION_H

#include "event_loop.h"
#include "net_address.h"
#include "channel.h"

namespace ssp::net {
class TcpConnection : public std::enable_shared_from_this<TcpConnection>{
public:
    TcpConnection(EventLoop *loop, std::string name, int sockFd, const InetAddress &locAddr,
        const InetAddress &peerAddr, void *ssl = nullptr);

    ~TcpConnection();

    void Established();

    void SetMessageCallback(MessageCallback cb);

    void SetCloseCallback(CloseCallback cb);

    void Close();

    int32_t Send(const void *buffer, int32_t length);

private:
    void HandleRead(const TimeStamp &stamp);
    void HandleWrite(const TimeStamp &stamp);
    void HandleClose(const TimeStamp &stamp);
    void HandleError(const TimeStamp &stamp);
    [[maybe_unused]] void HandleReadTimeout(const TimeStamp &stamp);
    void Destory();

private:
    std::unique_ptr<Socket> socket_{nullptr};
    std::unique_ptr<Channel> channel_{nullptr};
    EventLoop *loop_{nullptr};
    MessageCallback msgCb_{nullptr};
    CloseCallback  closeCb_{nullptr};
    std::string connName_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
}
#endif //SSP_TOOLS_TCP_CONNECTION_H
