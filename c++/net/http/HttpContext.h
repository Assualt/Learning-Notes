#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "base/Timestamp.h"
#include "base/copyable.h"
#include "net/Buffer.h"

using namespace muduo::base;
using namespace muduo::net;

class HttpContext : public copyable {
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    HttpContext()
        : m_state(kExpectRequestLine) {
    }

    // default copy-ctor, dtor and assignment are fine

    // return false if any error
    bool parseRequest(Buffer *buf, Timestamp receiveTime);

    bool gotAll() const {
        return m_state == kGotAll;
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

    HttpRequestParseState m_state;
    HttpRequest           m_request;
};
