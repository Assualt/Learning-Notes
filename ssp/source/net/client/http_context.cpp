//
// Created by 侯鑫 on 2024/1/10.
//

#include "http_context.h"
#include "http_utils.h"
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
    if (space != end && response_.SetMethod({start, static_cast<size_t>(space - start)})) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char *question = std::find(start, space, '?');
            if (question != space) {
                response_.SetPath(std::string(start, question));
                response_.SetQuery(std::string(question, space));
            } else {
                response_.SetPath(std::string(start, space));
            }

            start   = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    response_.SetReqVersion("HTTP/1.1");
                } else if (*(end - 1) == '0') {
                    response_.SetReqVersion("HTTP/1.0");
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
            response_.SetReqStatusMsg(std::string(start, end));
            response_.SetStatusCode(status_code);
            response_.SetReqVersion(str);
            return true;
        } catch (...) {
            return succeed;
        }
    }

    return succeed;
}

// return false if any error
bool HttpContext::ParseRequest(Buffer *buf, const TimeStamp& receiveTime) {
    bool ok      = true;
    bool hasMore = true;
    while (hasMore) {
        if (state_ == kExpectRequestLine) {
            const char *crlf = buf->findCRLF();
            if (crlf) {
                ok = ProcessRequestLine(buf->peek(), crlf);
                if (ok) {
                    response_.SetRecvTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (state_ == kExpectHeaders) {
            const char *crlf = buf->findCRLF();
            if (crlf) {
                if (!lastBuffer_.empty()) {
                    std::string singleHeader = lastBuffer_ + std::string(buf->peek(), crlf);
                    auto        pos          = singleHeader.find_first_of(':');
                    if (pos != std::string::npos) {
                        response_.AddHeader({singleHeader.c_str(), pos},
                                            {singleHeader.c_str(), singleHeader.size()});
                    }
                    lastBuffer_.clear();
                    buf->retrieveUntil(crlf + 2);
                    continue;
                }

                const char *colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    // Fixme
                    response_.AddHeader({buf->peek(), static_cast<size_t>(colon - buf->peek())}, {colon + 1, static_cast<size_t>(crlf - colon - 1)});
                } else {
                    // empty line, end of header
                    // FIXME:
                    SetContentLengthType();
                    if ((lengthType_ == kContentLength) && (contentLength_ == 0)) {
                        state_ = kGotEnd;
                        hasMore = false;
                    } else {
                        state_ = kExpectBody;
                    }
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                hasMore      = false;
                lastBuffer_ = {buf->peek(), buf->readableBytes()};
            }
        } else if (state_ == kExpectBody || state_ == kGotAll) {
            ParseBodyPart(buf);
            hasMore = false;
        }
    }
    buf->retrieveAll(); // 清空缓冲区
    return ok;
}

void HttpContext::SetContentLengthType()
{
    auto contentStr = response_.Get(ContentLength);
    if (!contentStr.empty()) {
        contentLength_ = std::strtol(contentStr.c_str(), nullptr, 10);
        lengthType_       = kContentLength;
    }
    contentStr = response_.Get(TransferEncoding);
    if (strcasecmp(contentStr.c_str(), "chunked") == 0) {
        lengthType_ = kContentChunked;
    }
}

void HttpContext::ParseBodyPart(Buffer *buf)
{
    if (buf == nullptr) {
        return;
    }
    if (lengthType_ == kContentLength) {
        ParseBodyByContentLength(buf);
    } else if (lengthType_ == kContentChunked) {
        logger.Debug("parse buffer with chunked");
        ParseBodyByChunkedBuffer(buf);
    }
}

void HttpContext::ParseBodyByContentLength(Buffer *buf) 
{
    auto reqLen = std::strtol(response_.Get(ContentLength).c_str(), nullptr, 10);
    if (reqLen == 0) {
        state_ = kGotEnd;
        return;
    }
    response_.AppendBody(buf->peek(), buf->readableBytes());
    auto recvLen = response_.GetBodySize();
    if (recvLen >= reqLen) {
        state_ = kGotEnd;
    }
}

void HttpContext::ParseBodyByChunkedBuffer(Buffer *buf)
{
    while (true) {
        if (chunkLeftSize_ != 0) {
            if (chunkLeftSize_ < buf->readableBytes()) {
                response_.AppendBody(buf->peek(), chunkLeftSize_);
                buf->retrieveUntil(buf->peek() + chunkLeftSize_ + 2);
                logger.Debug("recv left chunked size %d", chunkLeftSize_);
                chunkLeftSize_ = 0;
            } else {
                uint32_t recvSize = buf->readableBytes();
                response_.AppendBody(buf->peek(), buf->readableBytes());
                buf->retrieveUntil(buf->peek() + buf->readableBytes());
                logger.Debug("should recv size:%d, real recv size:%d, left:%d", chunkLeftSize_, recvSize,
                             chunkLeftSize_ - recvSize);
                chunkLeftSize_ -= recvSize;
                break;
            }
        }

        const char *crlf = buf->findCRLF();
        if (crlf == nullptr) {
            break;
        }

        auto size = HttpUtils::GetChunkSize(std::string(buf->peek(), crlf));
        if (size == -1) {
            break;
        }

        buf->retrieveUntil(crlf + 2);
        logger.Debug("recv chunked size:%d ", static_cast<int>(size));
        if (size == 0) {
            state_ = kGotEnd;
            break;
        }

        // recv buf < size.
        if (buf->readableBytes() < static_cast<int>(size)) {
            chunkLeftSize_ = size - buf->readableBytes();
            response_.AppendBody(buf->peek(), buf->readableBytes());
            logger.Debug("==>success insert into %d bytes left size:%d", buf->readableBytes(), chunkLeftSize_);
            buf->retrieveUntil(buf->peek() + buf->readableBytes());
            break;
        }

        response_.AppendBody(buf->peek(), size);
        logger.Debug("success insert into %d bytes", size);
        buf->retrieveUntil(buf->peek() + size + 2);
    }
}

void HttpContext::Reset() 
{
    state_ = kExpectRequestLine;
}