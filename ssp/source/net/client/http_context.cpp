//
// Created by 侯鑫 on 2024/1/10.
//

#include "http_context.h"
#include "base/log.h"
#include <algorithm>
#include <string>

using namespace ssp::base;
using namespace ssp::net;

bool HttpContext::ProcessRequestLine(const char *begin, const char *end)
{
    bool        succeed = false;
    const char *start   = begin;
    const char *space   = std::find(start, end, ' ');
    if (space != end && m_request.SetMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char *question = std::find(start, space, '?');
            if (question != space) {
                m_request.SetPath(std::string(start, question));
                m_request.SetQuery(std::string(question, space));
            } else {
                m_request.SetPath(std::string(start, space));
            }
//            m_request.SetRequestPath(UrlUtils::UrlDecode(std::string(start, space)));
            m_request.SetRequestPath(std::string(start, space));
            // m_request.setRequestPath(std::string(start, space));
            start   = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    m_request.SetHttpVersion("HTTP/1.1");
                } else if (*(end - 1) == '0') {
                    m_request.SetHttpVersion("HTTP/1.0");
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
            m_request.SetStatusMessage(std::string(start, end));
            m_request.SetStatusCode(status_code);
            m_request.SetHttpVersion(str);
            return true;
        } catch (...) {
            return succeed;
        }
    }

    return succeed;
}

// return false if any error
bool HttpContext::ParseRequest(Buffer *buf, TimeStamp receiveTime) {
    bool ok      = true;
    bool hasMore = true;
    while (hasMore) {
        if (m_state == kExpectRequestLine) {
            const char *crlf = buf->findCRLF();
            if (crlf) {
                ok = ProcessRequestLine(buf->peek(), crlf);
                if (ok) {
                    m_request.SetRecvTime(receiveTime);
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
                if (!m_lastBuffer.empty()) {
                    std::string singleHeader = m_lastBuffer + std::string(buf->peek(), crlf);
                    auto        pos          = singleHeader.find_first_of(':');
                    if (pos != std::string::npos) {
                        m_request.AddHeader(singleHeader.c_str(), singleHeader.c_str() + pos,
                                            singleHeader.c_str() + singleHeader.size());
                    }
                    m_lastBuffer.clear();
                    buf->retrieveUntil(crlf + 2);
                    continue;
                }

                const char *colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    // Fixme
                    m_request.AddHeader(buf->peek(), colon, crlf);
                } else {
                    // empty line, end of header
                    // FIXME:
                    SetContentLengthType();
                    if ((m_lenType == kContentLength) && (m_contentLength == 0)) {
                        m_state = kGotEnd;
                        hasMore = false;
                    } else {
                        m_state = kExpectBody;
                    }
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                hasMore      = false;
                m_lastBuffer = {buf->peek(), buf->readableBytes()};
            }
        } else if (m_state == kExpectBody) {
            // FIXME:
            ParseBodyPart(buf);
            hasMore = false;
        } else if (m_state == kGotAll) {
            ParseBodyPart(buf);
            hasMore = false;
        }
    }
    buf->retrieveAll(); // 清空缓冲区
    return ok;
}

void HttpContext::SetContentLengthType()
{
    auto contentStr = m_request.Get(ContentLength);
    if (!contentStr.empty()) {
        m_contentLength = std::strtol(contentStr.c_str(), nullptr, 10);
        m_lenType       = kContentLength;
    }
    contentStr = m_request.Get(TransferEncoding);
    if (strcasecmp(contentStr.c_str(), "chunked") == 0) {
        m_lenType = kContentChunked;
    }
}

void HttpContext::ParseBodyPart(Buffer *buf)
{
    if (buf == nullptr) {
        return;
    }
    if (m_lenType == kContentLength) {
        ParseBodyByContentLength(buf);
    } else if (m_lenType == kContentChunked) {
        logger.Debug("parse buffer with chunked");
        ParseBodyByChunkedBuffer(buf);
    }
}

void HttpContext::ParseBodyByContentLength(Buffer *buf) {
    auto reqLen = atol(m_request.Get(ContentLength).c_str());
    if (reqLen == 0) {
        m_state = kGotEnd;
        return;
    }
    m_request.appendBodyBuffer(buf->peek(), buf->readableBytes());
    auto recvLen = m_request.GetBodySize();
    if (recvLen >= reqLen) {
        m_state = kGotEnd;
    }
}

void HttpContext::ParseBodyByChunkedBuffer(Buffer *buf)
{
    while (true) {
        if (m_chunkLeftSize != 0) {
            if (m_chunkLeftSize < buf->readableBytes()) {
                m_request.appendBodyBuffer(buf->peek(), m_chunkLeftSize);
                buf->retrieveUntil(buf->peek() + m_chunkLeftSize + 2);
                logger.Debug("recv left chunked size %d", m_chunkLeftSize);
                m_chunkLeftSize = 0;
            } else {
                uint32_t recvSize = buf->readableBytes();
                m_request.appendBodyBuffer(buf->peek(), buf->readableBytes());
                buf->retrieveUntil(buf->peek() + buf->readableBytes());
                logger.Debug("should recv size:%d, real recv size:%d, left:%d", m_chunkLeftSize, recvSize,
                             m_chunkLeftSize - recvSize);
                m_chunkLeftSize -= recvSize;
                break;
            }
        }

        const char *crlf = buf->findCRLF();
        if (crlf == nullptr) {
            break;
        }

        long nBytes = utils::chunkSize(std::string(buf->peek(), crlf));
        if (static_cast<int>(nBytes) == -1) {
            break;
        }

        buf->retrieveUntil(crlf + 2);
        logger.Debug("recv chunked size:%d ", static_cast<int>(nBytes));
        if (nBytes == 0) {
            m_state = kGotEnd;
            break;
        }

        // recv buf < nBytes.
        if (buf->readableBytes() < static_cast<int>(nBytes)) {
            m_chunkLeftSize = nBytes - buf->readableBytes();
            m_request.appendBodyBuffer(buf->peek(), buf->readableBytes());
            logger.Debug("==>success insert into %d bytes left size:%d", buf->readableBytes(), m_chunkLeftSize);
            buf->retrieveUntil(buf->peek() + buf->readableBytes());
            break;
        }

        m_request.appendBodyBuffer(buf->peek(), nBytes);
        logger.Debug("success insert into %d bytes", nBytes);
        buf->retrieveUntil(buf->peek() + nBytes + 2);
    }
}