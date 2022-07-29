#include "mail_process.h"
#include "base/Logging.h"
#include "base/crypto/base64.h"
#include "mailenv.h"
#include "mailutils.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <map>
using namespace muduo::base;
using namespace mail;

TIgnoreCaseSet MailProcess::m_SupportMethods{"helo", "ehlo", "starttls", "mail from", "rcpt to", "data", "vrfy", "expn", "help", "rset", "quit", "auth login"};

MailProcess::MailProcess(MailContext &ctx)
    : m_mailContext(ctx) {
}

bool MailProcess::SupportCommand(const std::string &strMethod) {
    return m_SupportMethods.count(strMethod);
}

bool MailProcess::isValidMailBox(const std::string &strMailBox) {
    size_t nPos   = strMailBox.find("@");
    size_t nCount = std::count(strMailBox.begin(), strMailBox.end(), '@');
    return nPos != std::string::npos && nPos != 0 && nPos != strMailBox.size() && nCount == 1;
}

std::pair<std::string, std::string> MailProcess::getCommandVal(const std::string &strCmd) {
    std::string strKey, strVal;
    bool        bFindKey     = false;
    std::string RemoveString = Utils::trimRight(strCmd, std::string("\r\n"));
    for (size_t i = 0; i < RemoveString.size(); ++i) {
        if (!bFindKey && SupportCommand(strKey)) {
            bFindKey = true;
        } else if (!bFindKey) {
            strKey.push_back(tolower(RemoveString[ i ]));
        } else {
            if ((i + 1) < RemoveString.size() && RemoveString[ i ] == '\r' && RemoveString[ i + 1 ] == '\n')
                break;
            else if (!isblank(RemoveString[ i ]) && RemoveString[ i ] != ':')
                strVal.push_back(RemoveString[ i ]);
        }
    }
    if (bFindKey)
        return std::pair<std::string, std::string>(strKey, strVal);
    return std::pair<std::string, std::string>(strKey, "");
}

std::pair<std::string, std::string> MailProcess::SplitMailAddress(const std::string &strCmd) {
    bool        bFindAt = false;
    std::string strKey, strVal;
    for (size_t i = 0; i < strCmd.size(); ++i) {
        if (strCmd[ i ] == '@')
            bFindAt = true;
        else if (!bFindAt)
            strKey.push_back(strCmd[ i ]);
        else
            strVal.push_back(strCmd[ i ]);
    }
    if (bFindAt)
        return std::pair<std::string, std::string>(strKey, strVal);
    return std::pair<std::string, std::string>("", "");
}
MAIL_STATE MailProcess::process(MAIL_STATE state, const std::string &bufString, std::string &replyString) {
    using mailFunc                              = std::function<MAIL_STATE(const std::string &, std::string &)>;
    std::map<MAIL_STATE, mailFunc> stateFuncMap = {
        {HELO, [ this ](const std::string &buf, std::string &reply) { return onHELO(buf, reply); }},
        {EHLO, [ this ](const std::string &buf, std::string &reply) { return onEHLO(buf, reply); }},
        {AUTH, [ this ](const std::string &buf, std::string &reply) { return onAuth(buf, reply); }},
        {AUTHPASS, [ this ](const std::string &buf, std::string &reply) { return onAuthPass(buf, reply); }},
        {AUTHEND, [ this ](const std::string &buf, std::string &reply) { return onAuthEND(buf, reply); }},
        {MAILFROM, [ this ](const std::string &buf, std::string &reply) { return onMailFrom(buf, reply); }},
        {RCPTTO, [ this ](const std::string &buf, std::string &reply) { return onRcptTo(buf, reply); }},
        {DATA, [ this ](const std::string &buf, std::string &reply) { return onData(buf, reply); }},
        {DATARECEIVE, [ this ](const std::string &buf, std::string &reply) { return onDataReceive(buf, reply); }},
        {DATAFINISH, [ this ](const std::string &buf, std::string &reply) { return onDataFinish(buf, reply); }},
        {QUIT, [ this ](const std::string &buf, std::string &reply) { return onQuit(buf, reply); }},
        {REST, [ this ](const std::string &buf, std::string &reply) { return onRest(buf, reply); }},
    };

    auto itr = stateFuncMap.find(state);
    if (itr != stateFuncMap.end()) {
        return std::invoke(itr->second, bufString, std::ref(replyString));
    }

    return DISCONNECT;
}

MAIL_STATE MailProcess::onHELO(const std::string &bufString, std::string &replyString) {
    if (strncasecmp(bufString.c_str(), "ehlo ", 5) == 0)
        return onEHLO(bufString, replyString);
    else if (strncasecmp(bufString.c_str(), "helo ", 5) != 0) {
        replyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    }
    m_mailContext.m_strEhloDomain = bufString.substr(5, bufString.find("\r\n") - 5);
    replyString.assign("250 OK\r\n");
    return AUTH;
}

MAIL_STATE MailProcess::onEHLO(const std::string &bufString, std::string &replyString) {
    if (strncasecmp(bufString.c_str(), "ehlo ", 5) != 0) {
        replyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    }
    m_mailContext.m_strEhloDomain = bufString.substr(5, bufString.find("\r\n") - 5);
    replyString.assign("250-mail\r\n");
    replyString.append("250-PIPELINING\r\n");
    replyString.append("250-AUTH LOGIN\r\n");
    replyString.append("250-AUTH=LOGIN\r\n");
    replyString.append("250-STARTTLS\r\n");
    replyString.append("250-SMTPUTF8\r\n");
    replyString.append("250 8BITMIME\r\n");
    return AUTH;
}

MAIL_STATE MailProcess::onAuth(const std::string &bufString, std::string &replyString) {
    if (strncasecmp(bufString.c_str(), "mail from", 9) == 0) {
        return onMailFrom(bufString, replyString);
    } else if (strcasecmp(bufString.c_str(), "auth login\r\n") == 0) { // 250-AUTH login\r\n
        replyString.assign("354 dXNlcm5hbWU6\r\n");
        return AUTHPASS;
    } else if (strncasecmp(bufString.c_str(), "auth plain", 10) == 0) { // 250-AUTH PLAIN xxx:xxx\r\n
        return onAuthPLAIN(bufString, replyString);
    } else if (strncasecmp(bufString.c_str(), "auth login ", 11) == 0) { // 250-AUTH LOGIN xxx
        std::string tempString = Utils::trim(bufString.substr(11), std::string("\r\n"));
        return onAuthPass(tempString, replyString);
    }
    replyString.assign(SERVER_Response_UnSupportCommand);
    return DISCONNECT;
}

MAIL_STATE MailProcess::onAuthPass(const std::string &bufString, std::string &replyString) {
    replyString.assign("334 UGFzc3dvcmQ6\r\n");
    std::string strTempAuthUser = Utils::trimRight(bufString, std::string("\r\n"));
    strTempAuthUser             = Utils::trim(strTempAuthUser, std::string(" "));
    std::string     result;
    result.resize(100);
    base64::decoder decoder;
    auto            decodeSize = decoder.decode_str(strTempAuthUser.c_str(), strTempAuthUser.size(), result.data());
    if (decodeSize <= 0 || result.empty() || !isValidMailBox(result.data())) {
        logger.info("user is not base64 encode. invalid input auth user %s.", strTempAuthUser);
        replyString.assign("550 Auth Failed.\r\n");
        return DISCONNECT;
    }
    // check auth user is exists or not
    m_mailContext.m_strAuthUser = result;
    logger.info("auth login user:%s", m_mailContext.m_strAuthUser);
    return AUTHEND;
}

MAIL_STATE MailProcess::onAuthEND(const std::string &bufString, std::string &replyString) {
    std::string     strTempAuthPass = bufString.substr(0, bufString.size() - 2);
    std::string     result;
    result.resize(100);
    base64::decoder decoder;
    auto            decodeSize = decoder.decode_str(strTempAuthPass.c_str(), strTempAuthPass.size(), result.data());
    if (decodeSize <= 0 || result.empty()) {
        logger.info("pass is not base64 encode. invalid input Pass %s.", strTempAuthPass);
        replyString.assign("550 Auth User/Pass is invalid\r\n");
        return DISCONNECT;
    }
    // 这里查询后端接口 用于校验是否Auth OK.
    m_mailContext.m_strAuthPass = result;
    replyString.assign("235 Authentication successful\r\n");
    logger.info("get auth user:%s pass:%s", m_mailContext.m_strAuthUser, m_mailContext.m_strAuthPass);
    m_bAuthPassed = true;
    return MAILFROM;
}

MAIL_STATE MailProcess::onAuthPLAIN(const std::string &bufString, std::string &replyString) {
    std::string strCommand = Utils::trimRight(bufString, std::string("\r\n"));
    auto        UserPass   = Utils::split(strCommand, ' ');
    if (UserPass.size() != 3) {
        replyString.assign("502 Error.Syntax error.\r\n");
        return DISCONNECT;
    }
    std::string DecodeUserPass;
    int         nSize        = base64::decoder().decode_str(UserPass[ 2 ].c_str(), UserPass[ 2 ].size(), DecodeUserPass.data());
    auto        tempUserPass = Utils::split(DecodeUserPass, '\0');
    if (tempUserPass.size() < 2) {
        replyString.assign("502 Error.Syntax error.\r\n");
        return DISCONNECT;
    }
    m_mailContext.m_strAuthUser = tempUserPass[ tempUserPass.size() - 2 ];
    m_mailContext.m_strAuthPass = tempUserPass.back();
    logger.debug("decode %s byte %d --> auth user:%s auth pass:%s", bufString, nSize, m_mailContext.m_strAuthUser, m_mailContext.m_strAuthPass);
    replyString.assign("235 Authentication successful\r\n");
    m_bAuthPassed = true;
    return MAILFROM;
}

MAIL_STATE MailProcess::onMailFrom(const std::string &bufString, std::string &replyString) {
    if (strncasecmp(bufString.c_str(), "mail from:", 10) != 0) {
        replyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    }
    auto mailPair = getCommandVal(bufString);
    if (!SupportCommand(mailPair.first)) {
        replyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    } else if (strcasecmp(mailPair.first.c_str(), "mail from") != 0) {
        replyString.assign(SERVER_Response_BadSequence);
        return DISCONNECT;
    }

    if (mailPair.second.front() != '<' && mailPair.second.back() != '>') {
        logger.warning("invalid mail from:%s", mailPair.second);
        replyString.assign("550 invalid user" + mailPair.second + "\r\n");
        return DISCONNECT;
    }

    auto mailAddress = Utils::trimLeft(Utils::trimRight(mailPair.second, std::string(">")), std::string("<"));
    logger.info("mail from:%s", mailAddress);
    auto vec = SplitMailAddress(mailAddress);
    if (vec.first.empty() && vec.second.empty()) {
        logger.warning("invalid mail from: %s", mailAddress);
        replyString.assign("550 invalid user\r\n");
        return DISCONNECT;
    }
    if (MailEnv::getInstance().NeedAuth(vec.second) && !m_bAuthPassed) {
        replyString.assign("553 authentication is required\r\n");
        return DISCONNECT;
    }
    m_mailContext.m_strMailFrom = mailAddress;
    replyString.assign("250 Mail OK\r\n");
    return RCPTTO;
}

MAIL_STATE MailProcess::onRcptTo(const std::string &bufString, std::string &replyString) {
    auto command = getCommandVal(bufString);
    if (!SupportCommand(command.first)) {
        replyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    } else if (strcasecmp(command.first.c_str(), "data") == 0) {
        return onData(bufString, replyString);
    } else if (strcasecmp(command.first.c_str(), "rcpt to") != 0) {
        replyString.assign(SERVER_Response_BadSequence);
        return DISCONNECT;
    }
    if (command.second.front() != '<' && command.second.back() != '>') {
        logger.warning("invalid mail from:%s", command.second);
        replyString.assign("550 invalid user" + command.second + "\r\n");
        return DISCONNECT;
    }
    auto mailAddress = Utils::trimLeft(Utils::trimRight(command.second, std::string(">")), std::string("<"));
    auto vec         = SplitMailAddress(mailAddress);
    if (vec.first.empty() && vec.second.empty()) {
        logger.warning("invalid mail from: %s", mailAddress);
        replyString.assign("550 invalid user\r\n");
        return DISCONNECT;
    }
    logger.info("rcpt to:%s", mailAddress);
    if (MailEnv::getInstance().NeedAuth(vec.second) && !m_bAuthPassed) {
        replyString.assign("553 authentication is required\r\n");
        return DISCONNECT;
    }
    // check rcpt is belong to such server

    // check rcpt whether equals to the mail from
    if (m_mailContext.m_strMailFrom == mailAddress)
        m_mailContext.m_RcptSets.insert(mailAddress);
    replyString.assign("250 Mail OK\r\n");

    return RCPTTO;
}
MAIL_STATE MailProcess::onData(const std::string &bufString, std::string &replyString) {
    if (strcasecmp(bufString.c_str(), "data\r\n") != 0) {
        replyString.assign(SERVER_Response_BadSequence);
        return DISCONNECT;
    }
    replyString.assign("354 End data with <CR><LF>.<CR><LF>\r\n");
    return DATARECEIVE;
}

MAIL_STATE MailProcess::onDataReceive(const std::string &bufString, std::string &replyString) {
    // check 最后5个字符是否为 \r\n.\r\n
    m_mailContext.m_bodyBuf.append(bufString);
    if (strstr(m_mailContext.m_bodyBuf.peek(), "\r\n.\r\n") != nullptr) {
        return onDataFinish(bufString, replyString);
    }

    return DATARECEIVE;
}

MAIL_STATE MailProcess::onQuit(const std::string &bufString, std::string &replyString) {
    replyString.assign("221 bye\r\n");
    return DISCONNECT;
}

MAIL_STATE MailProcess::onDataFinish(const std::string &bufString, std::string &replyString) {
    replyString.assign("250 Mail OK queued as smtp with id \r\n");
    return QUIT;
}

MAIL_STATE MailProcess::onRest(const std::string &bufString, std::string &replyString) {
    replyString.assign("250 OK\r\n");
    m_mailContext.clearContext();
    return AUTH;
}
