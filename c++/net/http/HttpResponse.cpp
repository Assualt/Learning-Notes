#include "HttpResponse.h"
#include "base/Format.h"
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

void HttpResponse::appendToBuffer(Buffer &output) const {
    char buf[ 32 ];
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output.append(buf);
    output.append(statusMessage_);
    output.append("\r\n");

    if (encodingType_ == HttpContentType::ContentRaw) {
        output.append(FmtString("%: %\r\n").arg(ContentLength).arg(body_.size()).str());
    } else if (encodingType_ == HttpContentType::ContentStream) {
        output.append("Transfer-Encoding: chunked\r\n");
    }

    if (closeConnection_) {
        output.append(FmtString("%: close\r\n").arg(Connection).str());
    } else {
        output.append(FmtString("%: Keep-Alive\r\n").arg(Connection).str());
    }

    for (const auto &header : headers_) {
        output.append(header.first);
        output.append(": ");
        output.append(header.second);
        output.append("\r\n");
    }

    output.append(FmtString("%: %\r\n").arg(SERVER).arg(SERVERVal).str());
    output.append(CTRL);
    if (encodingType_ == HttpContentType::ContentRaw) {
        output.append(body_);
        output.append(CTRL);
    } else if (encodingType_ == HttpContentType::ContentStream) {
        output.append(bodyBuffer_.peek(), bodyBuffer_.readableBytes());
    }
}

void HttpResponse::setStatusMessage(HttpStatusCode statusCode, const std::string &httpVersion, const std::string &message, const std::string &strAcceptEncoding) {
    setStatusCode(statusCode);
    setStatusMessage(message);
    httpVersion_ = getHttpVersionByString(httpVersion);
}

void HttpResponse::setBodyStream(void *buf, size_t size, EncodingType type) {
    encodingType_                  = HttpContentType::ContentStream;
    size_t         compress_length = 0;
    MyStringBuffer outBuffer;
    if (type == Type_Gzip) {
        compress_length = ZlibStream::GzipCompress(reinterpret_cast<char *>(buf), size, outBuffer);
        addHeader(ContentEncoding, "gzip");
    } else if (type == Type_Deflate) {
        compress_length = ZlibStream::DeflateCompress(reinterpret_cast<char *>(buf), size, outBuffer);
        addHeader(ContentEncoding, "deflate");
    } else if (type == Type_Raw) {
        compress_length = ZlibStream::ZlibCompress(reinterpret_cast<char *>(buf), size, outBuffer);
        addHeader(ContentEncoding, "raw");
    }

    chunkedBuffer(outBuffer);
    logger.info("compress data with type:%d, compress data:%d, after:%d bodySize:%d", type, size, compress_length, bodyBuffer_.readableBytes());
}

void HttpResponse::chunkedBuffer(MyStringBuffer &buffer) {
    auto buf = std::make_unique<uint8_t[]>(CHUNK_SIZE);
    while (true) {
        size_t nRead = buffer.sgetn(reinterpret_cast<char *>(buf.get()), CHUNK_SIZE);
        if (nRead <= 0) {
            break;
        }
        std::stringstream ss;
        ss << std::hex << nRead;
        bodyBuffer_.append(ss.str().c_str(), ss.str().size());
        bodyBuffer_.append("\r\n", 2);
        bodyBuffer_.append(reinterpret_cast<char *>(buf.get()), nRead);
    }
    bodyBuffer_.append("\r\n0\r\n\r\n", 7);
}

void HttpResponse::printBodyBuffer() const {
    auto size = bodyBuffer_.readableBytes();
    for (auto i = 0; i < size; i++) {
        uint8_t ch = *(bodyBuffer_.peek() + i);
        printf("%02x ", ch);
        if ((ch == '\n') && (*(bodyBuffer_.peek() + i - 1) == '\r')) {
            printf("\n");
        }
    }
    printf("\n");
    printf("size:%lu\n", size);
}

void HttpResponse::appendToBodyBuffer(void *data, size_t len) {
    bodyBuffer_.append(data, len);
}

void HttpResponse::setBody(const Buffer &buf) {
    bodyBuffer_.append(buf.peek(), buf.readableBytes());
}