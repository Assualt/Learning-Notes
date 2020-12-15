#pragma once

#include <iostream>
#include <vector>
#include "httpconfig.h"
#include <sstream>
#include <string.h>
namespace http {
class HttpRequest {
public:
    typedef std::vector<std::pair<std::string, std::string>> ResourceMap;
    template <class T>
    void setHeader(const std::string &key, const T &val) {
        if (key == "Host")
            m_strRequestHost = utils::toString(val);
        else if (key == "Range")
            m_strRangeBytes = utils::toString(val);
        else
            m_vReqestHeader.push_back(std::pair<std::string, std::string>(key, utils::toString(val)));
    }

    std::string toStringHeader() {
        std::stringstream ss;
        ss << m_strRequestType << " " << m_strRequestPath << " " << m_strHttpVersion << CTRL;
        for (auto &item : m_vReqestHeader)
            ss << item.first << ": " << item.second << CTRL;
        if (!m_strRangeBytes.empty())
            ss << "Range: " << m_strRangeBytes << CTRL;
        ss << "Host: " << m_strRequestHost << CTRL;

        if (!m_strPostParams.empty())
            ss << CTRL << m_strPostParams;
        ss << CTRL;
        return ss.str();
    }
    friend std::ostream &operator<<(std::ostream &os, HttpRequest &obj) {
        os << "> " << obj.m_strRequestType << " " << obj.m_strRequestPath << " " << obj.m_strHttpVersion << CTRL;
        for (auto &item : obj.m_vReqestHeader)
            os << "> " << item.first << ": " << item.second << CTRL;
        if (!obj.m_strRangeBytes.empty())
            os << "Range: " << obj.m_strRangeBytes << CTRL;
        os << "> Host: " << obj.m_strRequestHost << CTRL;
        if (!obj.m_strPostParams.empty())
            os << obj.m_strPostParams << CTRL;
        return os;
    }

    std::string get(const std::string &key) {
        for (auto item : m_vReqestHeader) {
            if (strcasecmp(item.first.c_str(), key.c_str()) == 0)
                return item.second;
        }
        return "";
    }

    std::string getRequestType() const {
        return m_strRequestType;
    }
    void setRequestType(const std::string &strRequestType) {
        m_strRequestType = strRequestType;
    }
    std::string getHttpVersion() const {
        return m_strHttpVersion;
    }
    void setHttpVersion(const std::string &strHttpVersion) {
        m_strHttpVersion = strHttpVersion;
    }
    std::string getPostParams() const {
        return m_strPostParams;
    }
    void setPostParams(const std::string &strPostParams) {
        m_strPostParams = strPostParams;
    }
    std::string getRequestPath() const {
        return m_strRequestPath;
    }
    void setRequestPath(const std::string &strRequestPath) {
        m_strRequestPath = strRequestPath;
    }

private:
    std::string m_strRequestType;
    std::string m_strHttpVersion;
    std::string m_strPostParams;
    std::string m_strRequestPath;

    ResourceMap m_vReqestHeader;
    std::string m_strRequestHost;
    std::string m_strRangeBytes;
};
}  // namespace http