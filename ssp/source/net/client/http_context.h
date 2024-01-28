//
// Created by 侯鑫 on 2024/1/10.
//

#ifndef SSP_TOOLS_HTTP_CONTEXT_H
#define SSP_TOOLS_HTTP_CONTEXT_H

#include "base/timestamp.h"
#include "net/protocol/http_request.h"
#include "net/protocol/http_response.h"
#include "net/buffer.h"

using namespace ssp::base;

#define ContentLength "Content-Length"
#define TransferEncoding "Transfer-Encoding"
#define ContentEncoding "Content-Encoding"

namespace ssp::net {
class HttpContext {
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
        kGotEnd,
    };

    enum HttpRequestBodyLenType {
        kContentLength, kContentChunked
    };

    enum HttpRequestBodyEncoding {
        kEncodingGzip, kEncodingRaw, kEncodingBr, kEncodingZip
    };

    HttpContext()
        : state_(kExpectRequestLine), lengthType_(kContentLength) //, m_encodingType(kEncodingRaw)
        , contentLength_(-1) {}

    // return false if any error
    bool ParseRequest(Buffer *buf, const TimeStamp& receiveTime);

    [[nodiscard]] bool GotAll() const { return state_ == kGotAll; }

    [[nodiscard]] bool GotEnd() const { return state_ == kGotEnd; }

    void Reset();

    HttpResponse GetResponse() const { return response_; }

private:
    bool ProcessRequestLine(const char *begin, const char *end);

    void SetContentLengthType();

    void ParseBodyPart(Buffer *buf);

    void ParseBodyByContentLength(Buffer *buf);

    void ParseBodyByChunkedBuffer(Buffer *buf);

private:
    HttpRequestParseState state_;
    HttpRequestBodyLenType lengthType_;
    HttpResponse response_{};
    long contentLength_;
    long chunkLeftSize_{0};
    std::string lastBuffer_;
};

}
#endif //SSP_TOOLS_HTTP_CONTEXT_H
