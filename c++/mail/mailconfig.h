#pragma once

#include <iostream>
using namespace std;
namespace mail {
class MailConfigManager {
public:
    MailConfigManager() = default;
    bool initEx(const std::string& strConfigPath);

protected:
    std::string m_strConfigPath;
};
} // namespace mail