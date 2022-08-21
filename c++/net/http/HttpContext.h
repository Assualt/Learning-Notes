#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "base/Timestamp.h"
#include "base/copyable.h"
#include "net/Buffer.h"

using namespace muduo::base;
using namespace muduo::net;

#define ContentLength "Content-Length"
#define TransferEncoding "Transfer-Encoding"
#define ContentEncoding "Content-Encoding"

class HttpContext : public copyable {
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
        kGotEnd,
    };

    enum HttpRequestBodyLenType { kContentLength, kContentChunked };

    enum HttpRequestBodyEncoding { kEncodingGzip, kEncodingRaw, kEncodingBr, kEncodingZip };

    HttpContext()
        : m_state(kExpectRequestLine)
        , m_lenType(kContentLength)
        , m_encodingType(kEncodingRaw)
        , m_contentLength(-1) {
    }

    // return false if any error
    bool parseRequest(Buffer *buf, Timestamp receiveTime);

    bool gotAll() const {
        return m_state == kGotAll;
    }

    bool gotEnd() const {
        return m_state == kGotEnd;
    }

    void reset() {
        m_state = kExpectRequestLine;
        HttpRequest dummy;
    }

    const HttpRequest &request() const {
        return m_request;
    }

    HttpRequest &request() {
        return m_request;
    }

private:
    bool processRequestLine(const char *begin, const char *end);
    void setContentLengthType();

    void parseBodyPart(Buffer *buf);
    void parseBodyByContentLength(Buffer *buf);
    void parseBodyByChunkedBuffer(Buffer *buf);

private:
    HttpRequestParseState   m_state;
    HttpRequestBodyLenType  m_lenType;
    HttpRequestBodyEncoding m_encodingType;
    HttpRequest             m_request;
    long                    m_contentLength;
};
