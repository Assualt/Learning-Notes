#pragma once

#ifndef MAIL_ENV_H_2020_12_24
#define MAIL_ENV_H_2020_12_24

#define MAIL_VERSION_MAJOR 1
#define MAIL_VERSION_MINOR 0
#define MAIL_VERSION_PATCH 1

#define Welcome_Message(domain, build_date)                                                                            \
    "220 " + domain + " Mail TransPort Server for Free License (" + build_date + ")\r\n"
#define Welcome_Command(domain) "Mail Server Command Server " + domain + "\r\n.\r\n"
#define SERVER_Response_UnSupportCommand "500 UnSupported command\r\n"
#define SERVER_Response_BadSequence "503 Bad sequence of commands\r\n"

#define SERVER_APP_HELP_MESSGAE                                                                                        \
    "help       display the help\r\n"                                                                                  \
    "version    show mail server build version \r\n"                                                                   \
    "status     show mail server status\r\n.\r\n"

#include "base/Configure.h"
#include "base/Mutex.h"
#include <algorithm>
#include <set>
#include <string.h>
using namespace muduo::base;
namespace mail {
class MailEnv {

public:
    bool initMailEnv(ConfigureManager &config);
    bool Reload(ConfigureManager &config);

public:
    static MailEnv   &getInstance();

public:
    std::string getServerIP() const { return m_strServerIP; }

    int getServerPort() const { return m_nServerPort; }

    int getCommandPort() const { return m_nCommandPort; }

    int getMaxClient() const { return m_nMaxClients; }

    bool NeedAuth(const std::string &strDomain) {
        if (strDomain == m_strMailServerPrimaryDomain)
            return true;
        return false;
    }
    std::string getPrimaryDomain() const { return m_strMailServerPrimaryDomain; }

    std::string getBuildVersionDate() const { return m_strMailServerBuildDate; }

    std::string GenerateMsgTid();

protected:
    std::string m_strServerIP;
    int         m_nServerPort;
    int         m_nCommandPort;
    int         m_nMaxClients;
    std::string m_strMailServerPrimaryDomain;
    std::string m_strMailServerBuildDate;

    static RWLock m_rwLock;

protected:
    MailEnv();
};

} // namespace mail

#endif