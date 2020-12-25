#pragma once

#ifndef MAIL_SERVER_H_2020_12_24
#define MAIL_SERVER_H_2020_12_24

#include "mailconfig.h"
#include "mailenv.h"
#include "threadpool.h"
#include <iostream>
#include "mailcontext.h"

using namespace std;
namespace mail {
enum MAIL_STATE { HELO, EHLO, AUTH, AUTHPASS, AUTHEND, MAILFROM, RCPTTO, DATA, DATAFINISH, DISCONNECT };
class ConnectionInfo {
public:
    std::string m_strConnectIP;
    int         m_nPort;
    int         m_nClientFd;
    int         m_nFDFlag;
};

struct StringCaseCmp : std::binary_function<std::string, std::string, bool> {
public:
    bool operator()(const string &lhs, const string &rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str());
    }
};

using TIgnoreCaseSet = std::set<std::string>;

class MailProcess {
public:
    MailProcess(MailContext &pContext);
    MAIL_STATE                          process(MAIL_STATE state, const std::string &BufString, std::string &ReplyString);
    std::pair<std::string, std::string> getCommandVal(const std::string &strCmd);
    std::pair<std::string, std::string> getMailRcpt(const std::string &strCmd);
    static bool                         SupportCommand(const std::string &strMethod);
    static bool                         isValidMailBox(const std::string &strMailBox);
protected:
    MAIL_STATE onHELO(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onEHLO(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onAuth(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onAuthPass(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onAuthEND(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onMailFrom(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onRcptTo(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onData(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onDataFinish(const std::string &BufString, std::string &Replystring);

protected:
    std::string           m_strAuthUser;
    std::string           m_strAuthPass;
    bool                  m_bAuthPassed = false;
    static TIgnoreCaseSet m_SupportMethods;
    MailContext           m_MailContext;
};

class MailServer {
public:
    MailServer();
    void initEx(const std::string &strConfigPath);
    void startServer(size_t nThreadCount = 10);

public:
    static void HandleRequest(ConnectionInfo *info);

protected:
    bool WaitEvent(size_t nThreadCount);

protected:
    std::threadpool   m_MailQueueThreadsPool;
    MailConfigManager m_ConfigMgr;
    int               m_nMailServerSocket;
};

} // namespace mail

#endif
