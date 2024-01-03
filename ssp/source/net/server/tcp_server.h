//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_TCP_SERVER_H
#define SSP_TOOLS_TCP_SERVER_H

#include "net/comm_service.h"
#include "net/socket.h"
#include "net/acceptor.h"
#include "base/thread_pool.h"

using namespace ssp::base;
namespace ssp::net {
class TcpServer : public CommService {
public:
    TcpServer(const std::string &srvName, const InetAddress &address, bool isSsl = false);

    virtual ~TcpServer();

public:
    bool Init(const ConfigureManager &configure) override;

    bool Start() override;

    bool UnInit() override;

private:
    static void TcpServerInitCallback(uintptr_t);

private:
    std::unique_ptr<Acceptor> acceptor_{nullptr};
    std::unique_ptr<ThreadPool> pool_{nullptr};
    int32_t poolSize_;
};

}


#endif //SSP_TOOLS_TCP_SERVER_H
