#pragma once
#include "HttpConfig.h"
#include "HttpUtils.h"
#include "base/Logging.h"
#include "base/copyable.h"
#include "net/Buffer.h"
#include <map>
#include <sstream>

using namespace muduo::base;
using namespace muduo::net;

class HttpResponse : public copyable {
public:
    explicit HttpResponse(bool close)
        : statusCode_(kUnknown)
        , closeConnection_(close)
        , encodingType_(HttpContentType::ContentRaw) {}

    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }

    void setStatusMessage(HttpStatusCode statusCode, const std::string &httpVersion, const std::string &message,
                          const std::string &strAcceptEncoding = "");

    void setStatusMessage(const std::string &message) { statusMessage_ = message; }

    std::string getStatusMessage() const { return statusMessage_; }

    std::string getBody() const { return body_; }

    const Buffer &getBodyBuf() const { return bodyBuffer_; }

    void setCloseConnection(bool on) { closeConnection_ = on; }

    bool closeConnection() const { return closeConnection_; }

    void setContentType(const std::string &contentType) { addHeader(ContentType, contentType); }

    void printBodyBuffer() const;

    void chunkedBuffer(MyStringBuffer &buffer);

    // FIXME: replace std::string with std::stringPiece
    template <class T> void addHeader(const std::string &key, const T &val) {
        std::stringstream ss;
        ss << val;
        headers_.emplace(key, ss.str());
    }

    void setBody(const std::string &body) {
        encodingType_ = HttpContentType::ContentRaw;
        body_         = body;
    }

    void setBody(const Buffer &buf);

    void setBodyStream(void *buf, size_t size, EncodingType type);

    void appendToBuffer(Buffer &output) const;

    void appendToBodyBuffer(void *data, size_t len);

    HttpStatusCode getStatusCode() const { return statusCode_; }

    std::string getHeader(const std::string &str) {
        auto iter = headers_.find(str);
        return iter == headers_.end() ? "" : iter->second;
    }

    friend std::ostream &operator<<(std::ostream &os, const HttpResponse &obj) {
        os << "< " << getHttpVersionString(obj.httpVersion_) << " " << obj.statusCode_ << " " << obj.statusMessage_
           << CTRL;
        for (auto &item : obj.headers_)
            os << "< " << item.first << ": " << item.second << CTRL;
        os << CTRL;
        if (!obj.body_.empty()) {
            os << "[size:" << obj.body_.size() << "]" << obj.body_ << CTRL;
        } else {
            os << "[Binary]" << obj.bodyBuffer_.readableBytes() << CTRL;
        }
        return os;
    }

private:
    HttpVersion                             httpVersion_;
    std::multimap<std::string, std::string> headers_;
    HttpStatusCode                          statusCode_;
    std::string                             statusMessage_;
    bool                                    closeConnection_;
    std::string                             body_;
    HttpContentType                         encodingType_;
    Buffer                                  bodyBuffer_;
};
