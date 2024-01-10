//
// Created by 侯鑫 on 2024/1/3.
//

#include "http_request.h"
#include <set>

using namespace ssp::net;
using namespace ssp::base;

namespace {
inline bool isValidRequest(const std::string &requestType) {
    static std::set<std::string> reqTypeMap = {
        "GET", "POST", "PUT", "HEAD", "DELETE",
    };

    std::string type = requestType;
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);
    return reqTypeMap.find(type) != reqTypeMap.end();
}
}

int32_t HttpRequest::Append(const void *buf, int32_t size)
{
    return 0;
}

uint32_t HttpRequest::AppendToOutput(const void *buf, uint32_t size)
{
    return 0;
}

uint32_t HttpRequest::ParseInput(const void *buf, uint32_t size)
{
    return 0;
}

std::string HttpRequest::ToHeaderBuffer()
{
    return "";
}

std::string HttpRequest::ToStringHeader() const 
{
    std::stringstream ss;
    ss << requestType_ << " " << requestPath_ << " " << httpVersion_ << CTRL;
    ss << "Host: " << requestHost_ << CTRL;
    for (auto &item : requestHeader_)
        ss << item.first << ": " << item.second << CTRL;
    if (!m_strRangeBytes.empty())
        ss << "Range: " << m_strRangeBytes << CTRL;
    if (!requestParams_.empty())
        ss << CTRL << requestParams_;
    ss << CTRL;
    return ss.str();
}

void HttpRequest::SetParams(const std::map<std::string, std::string> &headerMap) {
    for (auto iter : headerMap) {
        m_urlQueryMap.insert(iter);
    }
    size_t nPos = requestPath_.find("?");
    if (nPos != std::string::npos) {
        std::string strKey, strVal;
        bool        bFindKey = false;
        for (int i = nPos + 1; i < requestPath_.size(); ++i) {
            if (requestPath_[ i ] == '=')
                bFindKey = true;
            else if (requestPath_[ i ] == '&') {
                bFindKey = false;
                m_urlQueryMap.insert({strKey, strVal});
                strKey = strVal = "";
            } else if (bFindKey) {
                strVal.push_back(requestPath_[ i ]);
            } else if (!bFindKey)
                strKey.push_back(requestPath_[ i ]);
        }
        if (!strKey.empty())
            m_urlQueryMap.insert({strKey, strVal});
    }
}

const std::string HttpRequest::Get(const std::string &key) const
{
    for (auto item : requestHeader_) {
        if (strcasecmp(item.first.c_str(), key.c_str()) == 0)
            return item.second;
    }
    return "";
}

std::string HttpRequest::GetRequestType() const
{
    return requestType_;
}

void HttpRequest::SetRequestType(const std::string &strRequestType)
{
    requestType_ = strRequestType;
}

std::string HttpRequest::GetHttpVersion() const
{
    return httpVersion_;
}

void HttpRequest::SetHttpVersion(const std::string &strHttpVersion)
{
    httpVersion_ = strHttpVersion;
}

std::string HttpRequest::GetPostParams() const
{
    return requestParams_;
}

void HttpRequest::SetPostParams(const std::string &strPostParams)
{
    requestParams_.append(strPostParams);
}

std::string HttpRequest::GetRequestPath() const
{
    return requestPath_;
}

void HttpRequest::SetRequestPath(const std::string &strRequestPath) {
    requestPath_     = strRequestPath;
    requestFilePath_ = strRequestPath;
    if (strRequestPath.find('?') != std::string::npos)
        requestFilePath_ = requestFilePath_.substr(0, requestFilePath_.find('?'));
}
std::string HttpRequest::GetRequestFilePath() const
{
    return requestFilePath_;
}

void HttpRequest::SetRequestFilePath(const std::string &strRequestFilePath)
{
//    requestFilePath_ = UrlUtils::UrlDecode(strRequestFilePath);
}

TimeStamp HttpRequest::GetRecvTime() const
{
    return m_recvTime;
}

void HttpRequest::SetRecvTime(const TimeStamp &time)
{
    m_recvTime = time;
}

std::string HttpRequest::GetPath() const
{
    return m_strPath;
}

void HttpRequest::SetPath(const std::string &path)
{
    m_strPath = path;
}

std::string HttpRequest::GetQuery() const
{
    return m_strQuery;
}

void HttpRequest::SetQuery(const std::string &query)
{
    m_strQuery = query;
}

std::string HttpRequest::GetStatusMessage() const
{
    return m_strStatusMessage;
}

void HttpRequest::SetStatusMessage(const std::string &message) { m_strStatusMessage = message; }

int HttpRequest::GetStatusCode() const { return m_statusCode; }

void HttpRequest::SetStatusCode(int statusCode) { m_statusCode = statusCode; }

bool HttpRequest::SetMethod(const char *start, const char *end) {
    std::string m(start, end);
    if (!isValidRequest(m)) {
        return false;
    }
    SetRequestType(m);
    return true;
}

std::string HttpRequest::GetParams(const std::string &key) const
{
    if (!m_urlQueryMap.count(key))
        return "";
    return m_urlQueryMap.at(key);
}
std::map<std::string, std::string> HttpRequest::GetAllParams() const
{
    return m_urlQueryMap;
}

void HttpRequest::AddHeader(const char *start, const char *colon, const char *end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
        ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && isspace(value[ value.size() - 1 ])) {
        value.resize(value.size() - 1);
    }
    AddHeader(field, value);
}

void HttpRequest::appendBodyBuffer(const void *buf, size_t size)
{
    bodyBuffer_.append(buf, size);
}

const Buffer &HttpRequest::GetBodyBuffer() const
{
    return bodyBuffer_;
}