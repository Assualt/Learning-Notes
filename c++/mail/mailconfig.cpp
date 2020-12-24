#include "mailconfig.h"

namespace mail {

bool MailConfigManager::initEx(const std::string &configPath) {
    m_strConfigPath = configPath;
    return true;
}
} // namespace mail