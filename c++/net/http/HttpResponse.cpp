#include "HttpResponse.h"
#include "base/Format.h"
#include <stdio.h>
using namespace muduo;
using namespace muduo::net;

void HttpResponse::appendToBuffer(Buffer *output) const {
    char buf[ 32 ];
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_);
    output->append("\r\n");

    if (encodingType_ == kContentRaw) {
        output->append(FmtString("Content-Length: %\r\n").arg(body_.size()).str().c_str());
    } else if (encodingType_ == kContentStream) {
        // output->append("Transfer-Encoding: chunked\r\n")
    }

    if (closeConnection_) {
        output->append("Connection: close\r\n");
    } else {
        output->append("Connection: Keep-Alive\r\n");
    }

    for (const auto &header : headers_) {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("server: httpserver 1.1\r\n");
    output->append("\r\n");
    if (encodingType_ == kContentRaw) {
        output->append(body_);
    } else if (encodingType_ == kContentStream) {
        output->append(bodyBuffer_.peek(), bodyBuffer_.readableBytes());
    }
}
void HttpResponse::setStatusMessage(int statusCode, const std::string &HttpVersion, const std::string &message, const std::string &strAcceptEncoding) {
    setStatusCode(static_cast<HttpStatusCode>(statusCode));
    setStatusMessage(message);
}