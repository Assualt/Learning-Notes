#pragma once

#include "mail_context.h"
#include "mail_env.h"
#include "net/TcpServer.h"
#include <iostream>
#include <map>
using namespace std;
using namespace muduo::net;
namespace mail {

struct StringCaseCmp : std::binary_function<std::string, std::string, bool> {
public:
    bool operator()(const string &lhs, const string &rhs) const { return strcasecmp(lhs.c_str(), rhs.c_str()); }
};

class MailServer {
public:
    MailServer(EventLoop &loop, const std::string &confPath);

    ~MailServer();

    void initEx(const std::string &strConfigPath);

    void startServer(size_t nThreadCount = 10);

    std::string getMailServerStatus();

private:
    void onConnection(const TcpConnectionPtr &conn);

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp stamp);

    void onCmdConnection(const TcpConnectionPtr &conn);

    void onCmdMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp stamp);

protected:
    std::shared_ptr<TcpServer>         server_{nullptr};
    std::shared_ptr<TcpServer>         cmdServer_{nullptr};
    std::shared_ptr<ConfigureManager>  cfgMgr_{nullptr};
    EventLoop &                        loop_;
    std::map<std::string, MailContext> mailCtx_;
};

} // namespace mail
