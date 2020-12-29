#pragma once

#ifndef MAIL_ENV_H_2020_12_24
#define MAIL_ENV_H_2020_12_24

#define MAIL_VERSION_MAJOR 1
#define MAIL_VERSION_MINOR 0
#define MAIL_VERSION_PATCH 1

#define Welcome_Message(domain, build_date) "220 " + domain + " Mail TransPort Server for Free License (" + build_date + ")\r\n"
#define SERVER_Response_UnSupportCommand "500 UnSupported command\r\n"
#define SERVER_Response_BadSequence "503 Bad sequence of commands\r\n"

#define SERVER_APP_HELP_MESSGAE                      \
    "help       display the help\r\n"                \
    "version    show mail server build version \r\n" \
    "status     show mail server status\r\n"

#include "configure.hpp"
#include <algorithm>
#include <mutex>
#include <set>
#include <string.h>
namespace mail {
class MailEnv {

public:
    bool initMailEnv(conf::ConfigureManager &config);
    bool Reload(conf::ConfigureManager &config);

public:
    static MailEnv &  getInstance();
    static MailEnv    g_MailEnvInstance;
    static std::mutex g_MutexLock;

public:
    std::string getServerIP() const {
        return m_strServerIP;
    }
    int getServerPort() const {
        return m_nServerPort;
    }
    int getCommandPort() const {
        return m_nCommandPort;
    }
    int getMaxClient() const {
        return m_nMaxClients;
    }
    bool NeedAuth(const std::string &strDomain) {
        if (strDomain == m_strMailServerPrimaryDomain)
            return true;
        return false;
    }
    std::string getPrimaryDomain() const {
        return m_strMailServerPrimaryDomain;
    }

    std::string getBuildVersionDate() const {
        return m_strMailServerBuildDate;
    }

protected:
    std::string m_strServerIP;
    int         m_nServerPort;
    int         m_nCommandPort;
    int         m_nMaxClients;
    std::string m_strMailServerPrimaryDomain;
    std::string m_strMailServerBuildDate;

protected:
    MailEnv();
};

} // namespace mail

#endif