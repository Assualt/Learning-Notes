//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_TCP_SERVER_H
#define SSP_TOOLS_TCP_SERVER_H

#include "net/comm_service.h"
#include "net/socket.h"
#include "net/acceptor.h"
#include "base/thread_pool.h"
#include "net/event_loop.h"
#include "net/tcp_connection.h"

using namespace ssp::base;
namespace ssp::net {
class TcpServer : public CommService {
public:
    TcpServer(EventLoop *loop, const std::string &srvName, const InetAddress &address, bool isSsl = false);

    ~TcpServer() override;

public:
    bool Init(const ConfigureManager &configure) override;

    bool Start() override;

    bool UnInit() override;

    void SetMessageCallback(MessageCallback fn);

private:
    static void TcpServerInitCallback(uintptr_t);

    void HandleNewConnection(int32_t fd, const InetAddress &address, void *args);

private:
    std::unique_ptr<Acceptor> acceptor_{nullptr};
    std::unique_ptr<ThreadPool> pool_{nullptr};
    int32_t poolSize_{0};
    std::map<std::string, TcpConnectionPtr> connectionMapper_;
    uint32_t connIndex_{1};
    MessageCallback msgCb_{nullptr};
};

}


#endif //SSP_TOOLS_TCP_SERVER_H
