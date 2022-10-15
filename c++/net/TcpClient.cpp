#include "TcpClient.h"
#include "SocketsOp.h"
#include "base/Logging.h"
#include <netdb.h>
using namespace muduo::net;
using namespace muduo::base;

TcpClient::TcpClient(bool useSsl)
    : useSsl_(useSsl) {
    fd_     = socket(AF_INET, SOCK_STREAM, 0);
    socket_ = std::make_unique<Socket>(fd_);
    if (useSsl) {
        socket_->initSSL();
    }
}

TcpClient::~TcpClient() { close(); }

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

bool TcpClient::connect(const std::string &host, uint16_t port, bool needSwitchSSL) {
    logger.info("begin to connect %s:%d ...", host, port);
    auto hosts = gethostbyname(host.c_str());
    if (hosts == nullptr) {
        logger.error("can't get host [%s:%d] ip yet.", host, port);
        return false;
    }

    for (size_t idx = 0; hosts->h_addr_list[ idx ]; ++idx) {
        sockaddr_in sock = {
            .sin_family = static_cast<sa_family_t>(hosts->h_addrtype),
            .sin_port   = htons(port),
            .sin_addr   = *(struct in_addr *)hosts->h_addr_list[ idx ],
        };

        auto address = InetAddress(sock);
        address.setHost(host);
        bool result = socket_->connect(address, connectTimeOut_);
        if (!result) {
            socket_->close();
            continue;
        }

        if (needSwitchSSL) {
            switchToSSLConnect(address);
        }

        return true;
    }

    logger.info("can't connect the url. %s:%d ", host, port);
    return false;
}

bool TcpClient::connect(const InetAddress &address) {
    bool ret = socket_->connect(address, connectTimeOut_);
    if (ret && useSsl_) {
        return switchToSSLConnect(address);
    }
    return ret;
}

int32_t TcpClient::sendRequest(const Buffer &buffer) {
    return socket_->write((void *)buffer.peek(), buffer.readableBytes());
}

int32_t TcpClient::sendBuf(const char *buf, uint32_t size) { return socket_->write((void *)buf, size); }

int32_t TcpClient::recvResponse(Buffer &recvBuf) { return socket_->read(recvBuf); }

void TcpClient::showTlsInfo() {
#ifdef USE_SSL
    socket_->showTlsInfo();
#endif
}

uint32_t TcpClient::switchToSSLConnect(const InetAddress &address) {
    if (socket_ == nullptr) {
        return UINT32_MAX;
    }
    return socket_->switchToSSL(true, address.host());
}
