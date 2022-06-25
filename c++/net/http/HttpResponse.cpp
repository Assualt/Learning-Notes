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

    if (encodingType_ == kContentRaw) {
        output.append(FmtString("Content-Length: %\r\n").arg(body_.size()).str().c_str());
    } else if (encodingType_ == kContentStream) {
        output.append("Transfer-Encoding: chunked\r\n");
    }

    if (closeConnection_) {
        output.append("Connection: close\r\n");
    } else {
        output.append("Connection: Keep-Alive\r\n");
    }

    for (const auto &header : headers_) {
        output.append(header.first);
        output.append(": ");
        output.append(header.second);
        output.append("\r\n");
    }

    output.append("server: httpserver 1.1\r\n");
    output.append("\r\n");
    if (encodingType_ == kContentRaw) {
        output.append(body_);
        output.append("\r\n");
    } else if (encodingType_ == kContentStream) {
        output.append(bodyBuffer_.peek(), bodyBuffer_.readableBytes());
    }
}

void HttpResponse::setStatusMessage(int statusCode, const std::string &HttpVersion, const std::string &message, const std::string &strAcceptEncoding) {
    setStatusCode(static_cast<HttpStatusCode>(statusCode));
    setStatusMessage(message);
}

void HttpResponse::setBodyStream(void *buf, size_t size, EncodingType type) {
    encodingType_                  = kContentStream;
    size_t         compress_length = 0;
    MyStringBuffer outbuffer;
    if (type == Type_Gzip) {
        compress_length = ZlibStream::GzipCompress(reinterpret_cast<char *>(buf), size, outbuffer);
        addHeader("Content-Encoding", "gzip");
    } else if (type == Type_Deflate) {
        compress_length = ZlibStream::DeflateCompress(reinterpret_cast<char *>(buf), size, outbuffer);
        addHeader("Content-Encoding", "defalte");
    } else if (type == Type_Raw) {
        compress_length = ZlibStream::ZlibCompress(reinterpret_cast<char *>(buf), size, outbuffer);
        addHeader("Content-Encoding", "raw");
    }

    chunkedBuffer(outbuffer);
    logger.info("compress data with type:%d, compress data:%d, after:%d bdoySize:%d", type, size, compress_length, bodyBuffer_.readableBytes());
}

void HttpResponse::chunkedBuffer(MyStringBuffer &buffer) {
    auto buf = std::make_unique<uint8_t[]>(CHUNK_SIGNLE_SIZE);
    while (true) {
        size_t nRead = buffer.sgetn(reinterpret_cast<char *>(buf.get()), CHUNK_SIGNLE_SIZE);
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
    printf("size:%d\n", size);
}