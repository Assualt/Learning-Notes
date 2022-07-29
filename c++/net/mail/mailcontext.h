#pragma once

#include "net/Buffer.h"
#include <iostream>
#include <set>
namespace mail {

enum MAIL_STATE : uint32_t {
    WELCOME,
    HELO,
    EHLO,
    AUTH,
    AUTHPASS,
    AUTHEND,
    MAILFROM,
    RCPTTO,
    DATA,
    DATARECEIVE,
    DATAFINISH,
    QUIT,
    DISCONNECT,
    REST,
};
class MailContext {
public:
    std::string m_strAuthUser;
    std::string m_strAuthPass;
    std::string m_strRequestIP;
    int         m_nRequestPort;

    std::string           m_strMailFrom;
    std::set<std::string> m_RcptSets;
    std::string           m_strEhloDomain;

    std::string        m_strSubject;
    MAIL_STATE         m_curStat{WELCOME};
    muduo::net::Buffer m_bodyBuf;

public:
    void clearContext() {
        m_strAuthPass.clear();
        m_strAuthUser.clear();
        m_strMailFrom.clear();
        m_RcptSets.clear();
        m_strSubject.clear();
    }
};
} // namespace mail