#include "HttpRequest.h"
#include "HttpUtils.h"

std::string HttpRequest::toStringHeader() const {
    std::stringstream ss;
    ss << m_strRequestType << " " << m_strRequestPath << " " << m_strHttpVersion << CTRL;
    ss << "Host: " << m_strRequestHost << CTRL;
    for (auto &item : m_vRequestHeader)
        ss << item.first << ": " << item.second << CTRL;
    if (!m_strRangeBytes.empty())
        ss << "Range: " << m_strRangeBytes << CTRL;
    if (!m_strPostParams.empty())
        ss << CTRL << m_strPostParams;
    ss << CTRL;
    return ss.str();
}

void HttpRequest::setParams(const std::map<std::string, std::string> &headerMap) {
    for (auto iter : headerMap) {
        m_urlQueryMap.insert(iter);
    }
    size_t nPos = m_strRequestPath.find("?");
    if (nPos != std::string::npos) {
        std::string strKey, strVal;
        bool        bFindKey = false;
        for (int i = nPos + 1; i < m_strRequestPath.size(); ++i) {
            if (m_strRequestPath[ i ] == '=')
                bFindKey = true;
            else if (m_strRequestPath[ i ] == '&') {
                bFindKey = false;
                m_urlQueryMap.insert({strKey, strVal});
                strKey = strVal = "";
            } else if (bFindKey) {
                strVal.push_back(m_strRequestPath[ i ]);
            } else if (!bFindKey)
                strKey.push_back(m_strRequestPath[ i ]);
        }
        if (!strKey.empty())
            m_urlQueryMap.insert({strKey, strVal});
    }
}

const std::string HttpRequest::get(const std::string &key) const {
    for (auto item : m_vRequestHeader) {
        if (strcasecmp(item.first.c_str(), key.c_str()) == 0)
            return item.second;
    }
    return "";
}

std::string HttpRequest::getRequestType() const { return m_strRequestType; }

void HttpRequest::setRequestType(const std::string &strRequestType) { m_strRequestType = strRequestType; }

std::string HttpRequest::getHttpVersion() const { return m_strHttpVersion; }

void HttpRequest::setHttpVersion(const std::string &strHttpVersion) { m_strHttpVersion = strHttpVersion; }

std::string HttpRequest::getPostParams() const { return m_strPostParams; }

void HttpRequest::setPostParams(const std::string &strPostParams) { m_strPostParams.append(strPostParams); }

std::string HttpRequest::getRequestPath() const { return m_strRequestPath; }

void HttpRequest::setRequestPath(const std::string &strRequestPath) {
    m_strRequestPath     = strRequestPath;
    m_strRequestFilePath = strRequestPath;
    if (strRequestPath.find("?") != std::string::npos)
        m_strRequestFilePath = m_strRequestFilePath.substr(0, m_strRequestFilePath.find("?"));
}
std::string HttpRequest::getRequestFilePath() const { return m_strRequestFilePath; }

void HttpRequest::setRequestFilePath(const std::string &strRequestFilePath) {
    m_strRequestFilePath = UrlUtils::UrlDecode(strRequestFilePath);
}

Timestamp HttpRequest::getRecvTime() const { return m_recvTime; }

void HttpRequest::setRecvTime(const Timestamp &time) { m_recvTime = time; }

std::string HttpRequest::getPath() const { return m_strPath; }

void HttpRequest::setPath(const std::string &path) { m_strPath = path; }

std::string HttpRequest::getQuery() const { return m_strQuery; }

void HttpRequest::setQuery(const std::string &query) { m_strQuery = query; }

std::string HttpRequest::getStatusMessage() const { return m_strStatusMessage; }

void HttpRequest::setStatusMessage(const std::string &message) { m_strStatusMessage = message; }

int HttpRequest::getStatusCode() const { return m_statusCode; }

void HttpRequest::setStatusCode(int statusCode) { m_statusCode = statusCode; }

bool HttpRequest::setMethod(const char *start, const char *end) {
    std::string m(start, end);
    if (!isValidRequest(m)) {
        return false;
    }
    setRequestType(m);
    return true;
}

std::string HttpRequest::getParams(const std::string &key) const {
    if (!m_urlQueryMap.count(key))
        return "";
    return m_urlQueryMap.at(key);
}
std::map<std::string, std::string> HttpRequest::getAllParams() const { return m_urlQueryMap; }

void HttpRequest::addHeader(const char *start, const char *colon, const char *end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
        ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && isspace(value[ value.size() - 1 ])) {
        value.resize(value.size() - 1);
    }
    setHeader(field, value);
}

void HttpRequest::appendBodyBuffer(const void *buf, size_t size) { m_bodyBuffer.append(buf, size); }

const muduo::net::Buffer &HttpRequest::getBodyBuffer() const { return m_bodyBuffer; }