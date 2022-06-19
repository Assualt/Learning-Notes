#include "HttpUtils.h"
#include "base/Logging.h"
#include "base/copyable.h"
#include "net/Buffer.h"
#include <map>
#include <sstream>

using namespace muduo::base;
using namespace muduo::net;

#define CHUNK_SIGNLE_SIZE 1024

class HttpResponse : public copyable {
public:
    enum HttpContentType {
        kContentRaw,   /* Content-Type: length */
        kContentStream /* Content-Bytes: bytes*/
    };
    enum EncodingType { Type_Gzip, Type_Br, Type_Deflate, Type_Raw };
    enum HttpStatusCode {
        kUnknown,
        k200Ok               = 200,
        k301MovedPermanently = 301,
        k400BadRequest       = 400,
        k404NotFound         = 404,
    };

    explicit HttpResponse(bool close)
        : statusCode_(kUnknown)
        , closeConnection_(close)
        , encodingType_(kContentRaw) {
    }

    void setStatusCode(HttpStatusCode code) {
        statusCode_ = code;
    }

    void setStatusMessage(int statusCode, const std::string &HttpVersion, const std::string &message, const std::string &strAcceptEncoding = "");

    void setStatusMessage(const std::string &message) {
        statusMessage_ = message;
    }

    void setCloseConnection(bool on) {
        closeConnection_ = on;
    }

    bool closeConnection() const {
        return closeConnection_;
    }

    void setContentType(const std::string &contentType) {
        addHeader("Content-Type", contentType);
    }

    void printBodyBuffer() const;

    void chunkedBuffer(MyStringBuffer &buffer);

    // FIXME: replace std::string with std::stringPiece
    template <class T> void addHeader(const std::string &key, const T &val) {
        std::stringstream ss;
        ss << val;
        headers_[ key ] = ss.str();
    }

    void setBody(const std::string &body) {
        encodingType_ = kContentRaw;
        body_         = body;
    }

    void setBodyStream(void *buf, size_t size, EncodingType type);

    void appendToBuffer(Buffer &output) const;

    HttpStatusCode getStatusCode() const {
        return statusCode_;
    }

private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode                     statusCode_;
    std::string                        statusMessage_;
    bool                               closeConnection_;
    std::string                        body_;
    HttpContentType                    encodingType_;
    Buffer                             bodyBuffer_;
};
