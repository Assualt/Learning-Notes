#include "mailenv.h"

namespace mail {
MailEnv    MailEnv::g_MailEnvInstance;
std::mutex MailEnv::g_MutexLock;

MailEnv::MailEnv()
    : m_strServerIP("127.0.0.1")
    , m_nServerPort(25)
    , m_nMaxClients(1000) {
}
MailEnv &MailEnv::getInstance() {
    std::lock_guard<std::mutex> lock(g_MutexLock);
    return g_MailEnvInstance;
}

bool MailEnv::initMailEnv(MailConfigManager &config) {

    return Reload(config);
}

bool MailEnv::Reload(MailConfigManager &config) {
    return true;
}

} // namespace mail