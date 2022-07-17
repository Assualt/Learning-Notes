#include "HttpConnection.h"
#include "base/Logging.h"
#include "base/System.h"
#include "net/InetAddress.h"
#include "net/Socket.h"
#include "netdb.h"

using namespace muduo::base;
using namespace muduo::net;
void HttpConnection::setTimeOut(int seconds) {
    timedOut_ = seconds;
}
bool HttpConnection::connect(const std::string &url) {
    return connect(HttpUrl(url));
}
bool HttpConnection::connect(const HttpUrl &url) {
    connectUrl_ = url.fullUrl;
    auto netloc = url.host;
    auto port   = url.port;
    if (url.scheme == "https") {
        port    = 443;
        useSsl_ = true;
    }

    logger.info("begin to connect %s:%d ...", netloc, port);

    auto host = gethostbyname(netloc.c_str());
    if (host == nullptr) {
        logger.error("can't get host [%s] ip yet.", netloc);
        return false;
    }

    int tmpFd = -1;
    for (size_t idx = 0; host->h_addr_list[ idx ]; ++idx) {
        tmpFd = socket(host->h_addrtype, SOCK_STREAM, 0);
        if (tmpFd == -1) {
            logger.info("create socket fd error errmsg:%s", System::GetErrMsg(errno));
            continue;
        }

        auto socket = std::make_shared<Socket>(tmpFd);
        if (socket == nullptr) {
            logger.info("create socket fd error errmsg:%s", System::GetErrMsg(errno));
            continue;
        }

        sockaddr_in sock = {.sin_family = static_cast<sa_family_t>(host->h_addrtype), .sin_port = htons(port), .sin_addr = *(struct in_addr *)host->h_addr_list[ idx ]};

        muduo::net::InetAddress addr(sock);
        bool result = socket->connect(addr, 10);
        if (!result) {
            socket->close();
            continue ;
        }

        return true;
    }


    logger.info("can't connect the server. %s ", url.netloc);
    return false;
}
