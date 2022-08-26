#include "HttpConnection.h"
#include "base/Logging.h"
#include "base/System.h"
#include "net/InetAddress.h"
#include "net/SocketsOp.h"
#include "netdb.h"

using namespace muduo::base;
using namespace muduo::net;

void HttpConnection::setTimeOut(int seconds) { timedOut_ = seconds; }

bool HttpConnection::connect(const std::string &url) {
    HttpUrl u(url);
    bool    ret = connect(u);
    LOG_IF(!ret).warning("can't connect to url:%s", url);
    return ret;
}

int32_t HttpConnection::send(const Buffer &reqBuf) {
    if (client_ == nullptr) {
        logger.info("socket is null, send error ");
        return -1;
    }

    return client_->sendRequest(reqBuf);
}

int32_t HttpConnection::recv(muduo::net::Buffer &respBuf) {
    if (client_ == nullptr) {
        logger.info("tcpClient is null, recv error ");
        return -1;
    }

    return client_->recvResponse(respBuf);
}

void HttpConnection::showTlsInfo() {
    client_->showTlsInfo();
}

bool HttpConnection::connect(const HttpUrl &url) {
    connectUrl_ = url.fullUrl;
    auto netloc = url.host;
    auto port   = url.port;
    if (url.scheme == "https") {
        useSsl_ = true;
    }

    logger.info("begin to connect %s:%d ...", netloc, port);

    auto host = gethostbyname(netloc.c_str());
    if (host == nullptr) {
        logger.error("can't get host [%s] ip yet.", netloc);
        return false;
    }

    for (size_t idx = 0; host->h_addr_list[ idx ]; ++idx) {
        auto tcpClient = std::make_unique<TcpClient>(useSsl_);
        if (tcpClient == nullptr) {
            logger.info("create socket fd error errmsg:%s", System::GetErrMsg(errno));
            continue;
        }

        sockaddr_in sock = {
            .sin_family = static_cast<sa_family_t>(host->h_addrtype),
            .sin_port   = htons(port),
            .sin_addr   = *(struct in_addr *)host->h_addr_list[ idx ],
        };

        tcpClient->setTimeOut(timedOut_, timedOut_, timedOut_);
        bool result = tcpClient->connect(muduo::net::InetAddress(sock));
        if (!result) {
            tcpClient->close();
            continue;
        }

        client_ = std::move(tcpClient);
        return true;
    }

    logger.info("can't connect the server. %s ", url.netloc);
    return false;
}
