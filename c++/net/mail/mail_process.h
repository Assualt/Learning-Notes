#pragma once
#include "mailcontext.h"
#include <memory>

namespace mail {
using TIgnoreCaseSet = std::set<std::string>;

class MailProcess {
public:
    MailProcess(MailContext &pContext);
    MAIL_STATE                          process(MAIL_STATE state, const std::string &bufString, std::string &replyString);
    std::pair<std::string, std::string> getCommandVal(const std::string &strCmd);
    std::pair<std::string, std::string> SplitMailAddress(const std::string &strCmd);
    static bool                         SupportCommand(const std::string &strMethod);
    static bool                         isValidMailBox(const std::string &strMailBox);

protected:
    MAIL_STATE onHELO(const std::string &bufString, std::string &replyString);
    MAIL_STATE onEHLO(const std::string &bufString, std::string &replyString);
    MAIL_STATE onAuth(const std::string &bufString, std::string &replyString);
    MAIL_STATE onAuthPass(const std::string &bufString, std::string &replyString);
    MAIL_STATE onAuthEND(const std::string &bufString, std::string &replyString);
    MAIL_STATE onAuthPLAIN(const std::string &bufString, std::string &replyString);
    MAIL_STATE onMailFrom(const std::string &bufString, std::string &replyString);
    MAIL_STATE onRcptTo(const std::string &bufString, std::string &replyString);
    MAIL_STATE onData(const std::string &bufString, std::string &replyString);
    MAIL_STATE onDataReceive(const std::string &bufString, std::string &replyString);
    MAIL_STATE onDataFinish(const std::string &bufString, std::string &replyString);
    MAIL_STATE onQuit(const std::string &bufString, std::string &replyString);
    MAIL_STATE onRest(const std::string &bufString, std::string &replyString);

protected:
    std::string           m_strAuthUser;
    std::string           m_strAuthPass;
    bool                  m_bAuthPassed = false;
    static TIgnoreCaseSet m_SupportMethods;
    MailContext          &m_mailContext;
    bool                  m_bDataRecving{false};
};

} // namespace mail
