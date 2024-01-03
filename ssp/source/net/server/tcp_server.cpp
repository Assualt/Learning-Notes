//
// Created by 侯鑫 on 2024/1/3.
//

#include "tcp_server.h"
#include "base/log.h"

using namespace ssp::net;

TcpServer::TcpServer(const std::string &srvName, const InetAddress &address, bool isSsl)
    :acceptor_(std::make_unique<Acceptor>(address, true))
{
    if (isSsl) {
        acceptor_->SwitchToSSL();
    }
}

TcpServer::~TcpServer()
{
    UnInit();
}

bool TcpServer::Init(const ConfigureManager &configure)
{
    poolSize_ = configure.GetInt(10, "/program/server/ThreadNum");
    pool_ = std::make_unique<ThreadPool>("tcp_server");
    if (pool_ == nullptr) {
        return false;
    }

    pool_->SetMaxQueueSize(configure.GetInt(10, "/program/server/QueueSize"));
    pool_->SetThreadInitCallBack(&TcpServer::TcpServerInitCallback);
    return false;
}

bool TcpServer::Start()
{
    pool_->Start(poolSize_, "TcpServer");
    logger.Info("TcpServer start is ok.....");
    return true;
}

bool TcpServer::UnInit()
{
    logger.Info("TcpServer is exiting now ...");
    return true;
}

void TcpServer::TcpServerInitCallback(uintptr_t val)
{
    logger.Debug("thread has been init ...");
}