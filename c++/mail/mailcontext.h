#pragma once

#include <iostream>
#include <set>
namespace mail {
class MailContext {
public:
    std::string m_strAuthUser;
    std::string m_strAuthPass;
    std::string m_strRequestIP;
    int         m_nRequestPort;

    std::string           m_strMailFrom;
    std::set<std::string> m_RcptSets;
    std::string           m_strEhloDomain;

    std::string m_strSubject;

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