#pragma once

#ifndef MAIL_SERVER_H_2020_12_24
#define MAIL_SERVER_H_2020_12_24

#include "configure.hpp"
#include "log.h"
#include "mailcontext.h"
#include "mailenv.h"
#include "threadpool.h"
#include <iostream>
#define SERVER_Trans_LOGGER "server.trans"
#define APP "App.command"

using namespace std;
namespace mail {
enum MAIL_STATE { HELO, EHLO, AUTH, AUTHPASS, AUTHEND, MAILFROM, RCPTTO, DATA, DATAFINISH, DISCONNECT, REST };
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
    std::pair<std::string, std::string> SplitMailAddress(const std::string &strCmd);
    static bool                         SupportCommand(const std::string &strMethod);
    static bool                         isValidMailBox(const std::string &strMailBox);

protected:
    MAIL_STATE onHELO(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onEHLO(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onAuth(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onAuthPass(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onAuthEND(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onAuthPLAIN(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onMailFrom(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onRcptTo(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onData(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onDataFinish(const std::string &BufString, std::string &ReplyString);
    MAIL_STATE onRest(const std::string &BufString, std::string &ReplyString);

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
    ~MailServer();
    std::string getMailServerStatus();

public:
    static void HandleRequest(ConnectionInfo *info);
    static void HandleCommandRequest(ConnectionInfo *info, MailServer *pServer);

protected:
    bool WaitEvent(size_t nThreadCount);
    bool StartCommandEvent(size_t nThreadCount);

protected:
    std::threadpool         m_MailQueueThreadsPool;
    std::threadpool         m_MailCommandThreadsPool;
    int                     m_nMailServerSocket;
    int                     m_nCommandListenSocket;
    conf::ConfigureManager *m_ptrConfigMgr;
    tlog::Logger *          m_ptrServerTransLogger;
};

} // namespace mail

#endif
