//
// Created by 侯鑫 on 2024/1/3.
//

#include "tcp_server.h"
#include "base/log.h"
#include "base/format.h"
#include "net/socket_ops.h"

using namespace ssp::net;
using namespace ssp::base;

TcpServer::TcpServer(EventLoop *loop, const std::string &srvName, const InetAddress &address, bool isSsl)
    :acceptor_(std::make_unique<Acceptor>(loop, address, true))
{
    if (isSsl) {
        acceptor_->SwitchToSSL();
    }
}

TcpServer::~TcpServer()
{
    log_sys.Info("TcpServer is exiting.... ");
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
    acceptor_->SetConnectionCallback([this](auto p1, auto p2, auto p3) { HandleNewConnection(p1, p2, p3); });
    return false;
}

bool TcpServer::Start()
{
    pool_->Start(poolSize_, "TcpServer");
    acceptor_->Listen();
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

void TcpServer::SetMessageCallback(MessageCallback fn)
{
    msgCb_ = std::move(fn);
}

void TcpServer::HandleNewConnection(int32_t fd, const ssp::net::InetAddress &address, void *args)
{
    logger.Info("new connection is already now...");
    auto loop = EventLoop::ApplyLoop();
    if (loop == nullptr) {
        logger.Warning("loop is busy. ");
        return;
    }

    InetAddress locAddr(sockets::GetLocalAddr(fd));
    std::string connName = FmtString("%-%#%").arg("tcpServer").arg(locAddr.ToIpPort()).arg(connIndex_++).str();
    auto ptr = std::make_shared<TcpConnection>(loop, connName, fd, locAddr, address, args);
    logger.Info("TcpServer::newConnection [%s] - new connection [%s] from [%s]", "TcpServer", connName,
                address.ToIpPort());
    connectionMapper_[connName] = ptr;
    ptr->SetMessageCallback(msgCb_);
    ptr->SetCloseCallback([this, connName]() {
        logger.Info("connection [%s] - released.", connName);
        connectionMapper_.erase(connName);
    });
    pool_->Run([loop, ptr](uintptr_t val) {
        ptr->Established();
        loop->Loop();
    });
}