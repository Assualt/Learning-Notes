#include "base/Logging.h"
#include "base/Timestamp.h"
#include "mail_env.h"
#include "mail_utils.h"
#include <uuid/uuid.h>

using namespace muduo::base;
namespace mail {

RWLock MailEnv::m_rwLock;

MailEnv::MailEnv()
    : m_strServerIP("127.0.0.1")
    , m_nServerPort(8025)
    , m_nCommandPort(8026)
    , m_nMaxClients(1000)
    , m_strMailServerPrimaryDomain("test.com") {
    m_strMailServerBuildDate = Timestamp::now().toFmtString();
}
MailEnv &MailEnv::getInstance() {
    static MailEnv env;
    return env;
}

std::string MailEnv::GenerateMsgTid() {
    auto buffer = std::make_unique<char[]>(128);
    uuid_generate(reinterpret_cast<unsigned char *>(buffer.get()));

    return buffer.get();
}

bool MailEnv::initMailEnv(ConfigureManager &config) {
    std::string strPrefix = "/programs/";
    config.changeAccessPath(strPrefix + "mailserver");
    m_strMailServerPrimaryDomain = config.getString("", "PrimaryDomain");
    m_strServerIP                = config.getString("127.0.0.1", "Host");
    m_nServerPort                = config.getInt(8025, "ServerPort");
    m_nMaxClients                = config.getInt(100, "AcceptClient");
    m_nCommandPort               = config.getInt(8026, "CommandPort");

    logger.debug("primaryDomain:%s, serverIP:%s:%d maxClients:%d cmd Port:%d", m_strMailServerPrimaryDomain,
                 m_strServerIP, m_nServerPort, m_nMaxClients, m_nCommandPort);
    return Reload(config);
}

bool MailEnv::Reload(ConfigureManager &config) {
    m_rwLock.WLock();
    m_rwLock.UnLock();
    return true;
}

} // namespace mail