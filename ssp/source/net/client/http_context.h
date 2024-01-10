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
            : m_state(kExpectRequestLine), m_lenType(kContentLength), m_encodingType(kEncodingRaw),
              m_contentLength(-1) {}

    // return false if any error
    bool ParseRequest(Buffer *buf, TimeStamp receiveTime);

    bool GotAll() const { return m_state == kGotAll; }

    bool GotEnd() const { return m_state == kGotEnd; }

    void Reset() {
        m_state = kExpectRequestLine;
    }

    [[nodiscard]] const HttpRequest &request() const { return m_request; }

    HttpRequest &Request() { return m_request; }

private:
    bool ProcessRequestLine(const char *begin, const char *end);

    void SetContentLengthType();

    void ParseBodyPart(Buffer *buf);

    void ParseBodyByContentLength(Buffer *buf);

    void ParseBodyByChunkedBuffer(Buffer *buf);

private:
    HttpRequestParseState m_state;
    HttpRequestBodyLenType m_lenType;
    HttpRequestBodyEncoding m_encodingType;
    HttpRequest m_request{};
    long m_contentLength;
    long m_chunkLeftSize{0};
    std::string m_lastBuffer;
};

}
#endif //SSP_TOOLS_HTTP_CONTEXT_H
