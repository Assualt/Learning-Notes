#pragma once

#include <iostream>
#define MAIL_VERSION_MAJOR 1
#define MAIL_VERSION_MINOR 0
#define MAIL_VERSION_PATCH 1

#define Welcome_Message(domain, build_date) "220 " + domain + " Mail TransPort Server for Free License (" + build_date + ")\r\n"
#define SERVER_Response_UnSupportCommand "500 UnSupported command\r\n"
#define SERVER_Response_BadSequence "503 Bad sequence of commands\r\n"
using namespace std;
namespace mail {
class MailConfigManager {
public:
    MailConfigManager() = default;
    bool initEx(const std::string &strConfigPath);

protected:
    std::string m_strConfigPath;
};
} // namespace mail