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
                    if ((m_lenType == kContentLength) && (m_contentLenth == 0)) {
                        m_state = kGotAll;
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
            m_state = kGotAll;
            hasMore = false;
        }
    }
    buf->retrieveAll(); // 清空缓冲区
    return ok;
}

void HttpContext::setContentLengthType() {
    auto contentStr = m_request.get(ContentLength);
    if (!contentStr.empty()) {
        m_contentLenth = atol(contentStr.c_str());
        m_lenType      = kContentLength;
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
        parseBodyByChunkedBuffer(buf);
    }
}

void HttpContext::parseBodyByContentLength(Buffer *buf) {
    m_request.setPostParams(std::string(buf->peek(), buf->peek() + buf->readableBytes()));
}
void HttpContext::parseBodyByChunkedBuffer(Buffer *buf) {
    while (true) {
        const char *crlf = buf->findCRLF();
        if (crlf == nullptr) {
            break;
        }
        size_t nBytes = utils::chunkSize(std::string(buf->peek(), crlf));
        buf->retrieveUntil(crlf + 2);
        if ((nBytes == 0) || (crlf == nullptr)) {
            break;
        }
        crlf = buf->findCRLF();
        if (crlf - buf->peek() == nBytes) {

        } else {
            logger.warning("wrong bytes chunked size:%#x, read size:%#x", nBytes, crlf - buf->peek());
        }
        buf->retrieveUntil(crlf + 2);
    }
}