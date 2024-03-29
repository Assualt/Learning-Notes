#include "base/Logging.h"
#include "mail_context.h"
#include "mail_process.h"
#include "mailserver.h"
#include "net/TcpConnection.h"
#include <memory>
#include <string.h>

using namespace mail;

MailServer::MailServer(EventLoop &loop, const std::string &confPath)
    : loop_(loop) {
    initEx(confPath);
}

MailServer::~MailServer() = default;

void MailServer::initEx(const std::string &strConfigPath) {
    cfgMgr_ = std::make_shared<ConfigureManager>(strConfigPath);
    cfgMgr_->Init();
    if (!MailEnv::getInstance().initMailEnv(*cfgMgr_)) {
        throw std::runtime_error("init mail env failed");
    }
}

void MailServer::onConnection(const TcpConnectionPtr &conn) {
    if (!conn->isConnected()) {
        return;
    }
    logger.info("connection is ready. %s", conn->peerAddress().toIpPort());
    std::string welMsg =
        Welcome_Message(MailEnv::getInstance().getPrimaryDomain(), MailEnv::getInstance().getBuildVersionDate());
    conn->send(welMsg.c_str(), welMsg.size());
    if (mailCtx_.find(conn->name()) == mailCtx_.end()) {
        auto ctx      = MailContext();
        ctx.m_curStat = HELO;
        mailCtx_.emplace(conn->name(), std::move(ctx));
    }
}

void MailServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp) {
    auto mtx = mailCtx_.find(conn->name());
    if (mtx == mailCtx_.end()) {
        logger.info("mail context is mismatch");
        conn->shutdown();
        return;
    }

    MailProcess process(mtx->second);
    std::string replyStr;
    mtx->second.m_curStat = process.process(mtx->second.m_curStat, {buf->peek(), buf->readableBytes()}, replyStr);
    buf->retrieve(buf->readableBytes());
    conn->send(replyStr.c_str(), replyStr.size());
    if (mtx->second.m_curStat == MAIL_STATE::DISCONNECT) {
        mailCtx_.erase(conn->name());
        conn->shutdown();
    }
}

void MailServer::onCmdConnection(const TcpConnectionPtr &conn) {
    if (!conn->isConnected()) {
        return;
    }
    logger.info("[CMD]connection is ready. %s", conn->peerAddress().toIpPort());
    std::string welMsg = Welcome_Command(MailEnv::getInstance().getPrimaryDomain());
    conn->send(welMsg.c_str(), welMsg.size());
}

void MailServer::onCmdMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp) {
    auto cmd = std::string(buf->peek(), buf->readableBytes());
    logger.info("recv buf is ==> %s", cmd);
    // 命令处理
    conn->shutdown();
}

void MailServer::startServer(size_t nThreadCount) {
    auto &      env = MailEnv::getInstance();
    InetAddress address(env.getServerPort());
    server_ = std::make_shared<TcpServer>(&loop_, address, "mail server");
    server_->SetConnectionCallback([this](auto conn) { onConnection(conn); });
    server_->SetMessageCallback([this](auto conn, auto buf, auto stamp) { onMessage(conn, buf, stamp); });
    server_->SetThreadNum(nThreadCount);
    logger.info("mail server start at %s....", address.toIpPort());
    server_->Start();

    InetAddress cmdAddr(env.getCommandPort());
    cmdServer_ = std::make_shared<TcpServer>(&loop_, cmdAddr, "command Server");
    cmdServer_->SetConnectionCallback([this](auto conn) { onCmdConnection(conn); });
    cmdServer_->SetMessageCallback([this](auto conn, auto buf, auto stamp) { onCmdMessage(conn, buf, stamp); });

    logger.info("mail cmd server start at %s...", cmdAddr.toIpPort());
    cmdServer_->Start();
}

std::string MailServer::getMailServerStatus() { return ""; }