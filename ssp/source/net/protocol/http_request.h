//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_HTTP_REQUEST_H
#define SSP_TOOLS_HTTP_REQUEST_H

#include "protocol_msg.h"
#include "base/timestamp.h"
#include "net/buffer.h"
#include <iostream>
#include <sstream>
#include <iosfwd>
#include <map>

#define CTRL "\r\n"

namespace ssp::net {
class HttpRequest : protected protocol::ProtocolMessage {
protected:
    uint32_t AppendToOutput(const void *buf, uint32_t size) override;

    uint32_t ParseInput(const void *buf, uint32_t size) override;

    int32_t Append(const void *buf, int32_t size);

private:
    using Headers = std::vector<std::pair<std::string, std::string>>;

    std::string requestType_;
    std::string httpVersion_;
    std::string requestPath_;
    std::string requestFilePath_;
    Headers requestHeader_;
    std::string requestParams_;
    
    std::string                        requestHost_;
    std::string                        m_strRangeBytes;
    std::map<std::string, std::string> m_urlQueryMap;
    ssp::base::TimeStamp               m_recvTime;
    std::string                        m_strPath;
    std::string                        m_strQuery;
    int                                m_statusCode;
    std::string                        m_strStatusMessage;
    Buffer                 bodyBuffer_;

public:
    void SetRequestType(const std::string &type);

    void SetRequestPath(const std::string &path);

    template <class T> void AddHeader(const std::string &key, const T &val)
    {
        std::string tmpVal = std::stringstream(val).str();
        requestHeader_.push_back({key, tmpVal});
    }

    std::string ToHeaderBuffer();

    bool SetMethod(const char *from, const char *to);

    const std::string Get(const std::string &key) const;

    std::string ToStringHeader() const;

    std::string GetRequestType() const;

    std::string GetHttpVersion() const;

    void SetHttpVersion(const std::string &strHttpVersion);

    std::string GetPostParams() const;

    void SetPostParams(const std::string &strPostParams);

    std::string GetRequestPath() const;

    std::string GetRequestFilePath() const;

    void SetRequestFilePath(const std::string &strRequestFilePath);

    ssp::base::TimeStamp GetRecvTime() const;

    void SetRecvTime(const ssp::base::TimeStamp &time);

    std::string GetQuery() const;

    void SetQuery(const std::string &query);

    std::string GetPath() const;

    void SetPath(const std::string &path);

    std::string GetStatusMessage() const;

    void SetStatusMessage(const std::string &message);

    int GetStatusCode() const;

    void SetStatusCode(int statusCode);

    void AddHeader(const char *start, const char *colon, const char *end);

    void appendBodyBuffer(const void *buf, size_t size);

    const Buffer &GetBodyBuffer() const;

    long GetBodySize() { return bodyBuffer_.readableBytes(); }

    // request 请求解析时候的 query参数解析
    std::map<std::string, std::string> &GetRequestQueryMap() { return m_urlQueryMap; }

    // request 请求的header列表
    const Headers &GetRequestHeader() { return requestHeader_; }

    friend std::ostream &operator<<(std::ostream &os, const HttpRequest &obj) {
        os << "> " << obj.requestType_ << " " << obj.requestPath_ << " " << obj.httpVersion_ << CTRL;
        os << "> Host: " << obj.requestHost_ << CTRL;
        for (auto &[key, val] : obj.GetAllParams())
            os << "> " << key << ": " << val << CTRL;
        if (!obj.m_strRangeBytes.empty())
            os << "Range: " << obj.m_strRangeBytes << CTRL;
        os << CTRL;
        if (!obj.bodyBuffer_.readableBytes()) {
            os << obj.bodyBuffer_.peek() << CTRL;
        } else if (obj.bodyBuffer_.readableBytes() != 0) {
            os << "[Binary]" << obj.bodyBuffer_.readableBytes() << CTRL;
        } else {
            os << "[null request body]" << CTRL;
        }

        return os;
    }

    void SetParams(const std::map<std::string, std::string> &headerMap);

    std::string GetParams(const std::string &key) const;

    std::map<std::string, std::string> GetAllParams() const;

};

}

#endif //SSP_TOOLS_HTTP_REQUEST_H
