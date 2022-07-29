#include "mailenv.h"
#include "mailutils.h"
#include <time.h>
namespace mail {
MailEnv    MailEnv::g_MailEnvInstance;
std::mutex MailEnv::g_MutexLock;

MailEnv::MailEnv()
    : m_strServerIP("127.0.0.1")
    , m_nServerPort(8025)
    , m_nCommandPort(8026)
    , m_nMaxClients(1000)
    , m_strMailServerPrimaryDomain("test.com") {
    time_t     t    = time(nullptr);
    struct tm *info = localtime(&t);
    char       temp[ 1024 ];
    size_t     nSize = strftime(temp, 1024, "%Y-%m-%d %H:%M:%S %Z", info);
    m_strMailServerBuildDate.assign(std::string(temp, nSize));
}
MailEnv &MailEnv::getInstance() {
    std::lock_guard<std::mutex> lock(g_MutexLock);
    return g_MailEnvInstance;
}

bool MailEnv::initMailEnv(ConfigureManager &config) {

    std::string strPrefix = "/programs/";
    config.changeAccessPath(strPrefix + "mailserver");
    m_strMailServerPrimaryDomain = config.getString("", "PrimaryDomain");
    m_strServerIP                = config.getString("127.0.0.1", "Host");
    m_nServerPort                = config.getInt(8025, "ServerPort");
    m_nMaxClients                = config.getInt(100, "AcceptClient");
    m_nCommandPort               = config.getInt(8026, "CommandPort");
    return Reload(config);
}

bool MailEnv::Reload(ConfigureManager &config) {
    return true;
}

} // namespace mail