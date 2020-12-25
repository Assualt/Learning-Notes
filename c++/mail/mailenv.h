#pragma once

#ifndef MAIL_ENV_H_2020_12_24
#define MAIL_ENV_H_2020_12_24
#include "mailconfig.h"
#include <algorithm>
#include <mutex>
#include <set>
#include <string.h>
namespace mail {
class MailEnv {

public:
    bool initMailEnv(MailConfigManager &config);
    bool Reload(MailConfigManager &config);

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
    std::string    m_strServerIP;
    int            m_nServerPort;
    int            m_nMaxClients;
    std::string    m_strMailServerPrimaryDomain;
    std::string    m_strMailServerBuildDate;

protected:
    MailEnv();
};

} // namespace mail

#endif