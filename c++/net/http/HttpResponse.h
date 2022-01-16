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
    enum HttpContentType {
        kContentRaw, /* Content-Type: length */
        kContentStream
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

    void setBodyStream(MyStringBuffer &buf, EncodingType type) {
        encodingType_ = kContentStream;
        buf.seekReadPos(0);
        std::unique_ptr<char[]> bodyBuffer(new char[ buf.size() ]);
        buf.sgetn(bodyBuffer.get(), buf.size());
        size_t compress_length = 0;
        if (type == Type_Gzip) {
            compress_length = ZlibStream::GzipCompress(bodyBuffer.get(), buf.size(), bodyBuffer_);
            addHeader("Content-Encoding", "gzip");
        } else if (type == Type_Deflate) {
            compress_length = ZlibStream::DeflateCompress(bodyBuffer.get(), buf.size(), bodyBuffer_);
            addHeader("Content-Encoding", "defalte");
        } else if (type == Type_Raw) {
            compress_length = ZlibStream::ZlibCompress(bodyBuffer.get(), buf.size(), bodyBuffer_);
            addHeader("Content-Encoding", "raw");
        }
        logger.info("compress data with type:%d, compress data:%d, after:%d", type, buf.size(), compress_length);
    }

    void appendToBuffer(Buffer *output) const;

    HttpStatusCode getStatusCode() const {
        return statusCode_;
    }

private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode                     statusCode_;
    // FIXME: add http version
    std::string     statusMessage_;
    bool            closeConnection_;
    std::string     body_;
    HttpContentType encodingType_;
    Buffer          bodyBuffer_;
};
