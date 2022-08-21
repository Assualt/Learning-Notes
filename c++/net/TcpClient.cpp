#include "TcpClient.h"
#include "base/Logging.h"
using namespace muduo::net;
using namespace muduo::base;

TcpClient::TcpClient(bool useSsl)
    : useSsl_(useSsl) {
    fd_     = socket(AF_INET, SOCK_STREAM, 0);
    socket_ = std::make_unique<Socket>(fd_);
    if (useSsl) {
        socket_->initSSL();
        logger.info("use ssl to connect ...");
    }
}

TcpClient::~TcpClient() {
    close();
}

void TcpClient::setTimeOut(int connectTimeout, int sendTimeOut, int recvTimeOut) {
    connectTimeOut_ = connectTimeout;
    sendTimeOut_    = sendTimeOut;
    readTimeOut_    = recvTimeOut;
    auto ret        = socket_->setReadTimeout(readTimeOut_);
    ret |= socket_->setWriteTimeout(sendTimeOut_);
    LOG_IF(ret != 0).info("socket set timeout(R:%d, W:%d) failed", readTimeOut_, sendTimeOut_);
}

void TcpClient::close() {
    if (useSsl_) {
        socket_->sslDisConnect();
    }
    socket_->close();
}

bool TcpClient::connect(const InetAddress &address) {
    bool ret = socket_->connect(address, connectTimeOut_);
    if (ret && useSsl_) {
        return socket_->switchToSSL();
    }
    return ret;
}

int32_t TcpClient::sendRequest(const Buffer &buffer) {
    return socket_->write((void *)buffer.peek(), buffer.readableBytes());
}

int32_t TcpClient::sendBuf(const char *buf, uint32_t size) {
    return socket_->write((void *)buf, size);
}

int32_t TcpClient::recvResponse(Buffer &recvBuf) {
    return socket_->read(recvBuf);
}
