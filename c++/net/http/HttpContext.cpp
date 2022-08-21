#include "HttpContext.h"
#include "HttpUtils.h"
using namespace muduo::base;
using namespace muduo::net;

bool HttpContext::processRequestLine(const char *begin, const char *end) {
    bool        succeed = false;
    const char *start   = begin;
    const char *space   = std::find(start, end, ' ');
    if (space != end && m_request.setMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char *question = std::find(start, space, '?');
            if (question != space) {
                m_request.setPath(std::string(start, question));
                m_request.setQuery(std::string(question, space));
            } else {
                m_request.setPath(std::string(start, space));
            }
            m_request.setRequestPath(UrlUtils::UrlDecode(std::string(start, space)));
            // m_request.setRequestPath(std::string(start, space));
            start   = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    m_request.setHttpVersion("HTTP/1.1");
                } else if (*(end - 1) == '0') {
                    m_request.setHttpVersion("HTTP/1.0");
                } else {
                    succeed = false;
                }
            }
        }
    } else if (start != space) {
        auto str = std::string(start, space);
        start    = space + 1;
        space    = std::find(start, end, ' ');
        try {
            int status_code = std::stoi(std::string(start, space));
            start           = space + 1;
            m_request.setStatusMessage(std::string(start, end));
            m_request.setStatusCode(status_code);
            m_request.setHttpVersion(str);
            return true;
        } catch (...) {
            return succeed;
        }
    }

    return succeed;
}

// return false if any error
bool HttpContext::parseRequest(Buffer *buf, Timestamp receiveTime) {
    bool ok      = true;
    bool hasMore = true;
    while (hasMore) {
        if (m_state == kExpectRequestLine) {
            const char *crlf = buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    m_request.setRecvTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);
                    m_state = kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (m_state == kExpectHeaders) {
            const char *crlf = buf->findCRLF();
            if (crlf) {
                const char *colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    m_request.addHeader(buf->peek(), colon, crlf);
                } else {
                    // empty line, end of header
                    // FIXME:
                    setContentLengthType();
                    if ((m_lenType == kContentLength) && (m_contentLength == 0)) {
                        m_state = kGotEnd;
                        hasMore = false;
                    } else {
                        m_state = kExpectBody;
                    }
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                hasMore = false;
            }
        } else if (m_state == kExpectBody) {
            // FIXME:
            parseBodyPart(buf);
            hasMore = false;
        } else if (m_state == kGotAll) {
            parseBodyPart(buf);
            hasMore = false;
        }
    }
    buf->retrieveAll(); // 清空缓冲区
    return ok;
}

void HttpContext::setContentLengthType() {
    auto contentStr = m_request.get(ContentLength);
    if (!contentStr.empty()) {
        m_contentLength = atol(contentStr.c_str());
        m_lenType       = kContentLength;
    }
    contentStr = m_request.get(TransferEncoding);
    if (strcasecmp(contentStr.c_str(), "chunked") == 0) {
        m_lenType = kContentChunked;
    }
}

void HttpContext::parseBodyPart(Buffer *buf) {
    if (buf == nullptr) {
        return;
    }
    if (m_lenType == kContentLength) {
        parseBodyByContentLength(buf);
    } else if (m_lenType == kContentChunked) {
        logger.info("parse buffer with chunked");
        parseBodyByChunkedBuffer(buf);
        m_state = kGotEnd;
    }
}

void HttpContext::parseBodyByContentLength(Buffer *buf) {
    auto reqLen = atol(m_request.get(ContentLength).c_str());
    if (reqLen == 0) {
        m_state = kGotEnd;
        return;
    }
    m_request.appendBodyBuffer(buf->peek(), buf->readableBytes());
    auto recvLen = m_request.getBodySize();
    logger.info("content length:%d recv length:%d", reqLen, recvLen);
    if (recvLen >= reqLen) {
        m_state = kGotEnd;
    }
}
void HttpContext::parseBodyByChunkedBuffer(Buffer *buf) {
    while (true) {
        const char *crlf = buf->findCRLF();
        if (crlf == nullptr) {
            break;
        }
        size_t nBytes = utils::chunkSize(std::string(buf->peek(), crlf));
        logger.info("recv chunked size:%#x", nBytes);
        buf->retrieveUntil(crlf + 2);
        if ((nBytes == 0) || (buf->readableBytes() < nBytes)) {
            break;
        }
        m_request.appendBodyBuffer(buf->peek(), nBytes);
        logger.info("success insert into %x bytes", nBytes);
        buf->retrieveUntil(crlf + 2);
    }
}