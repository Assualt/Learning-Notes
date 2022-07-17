#include "Socket.h"
#include "SocketsOp.h"
#include "base/Logging.h"
#include "net/InetAddress.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
namespace muduo {
namespace net {

Socket::Socket(int sockFd)
    : sockFd_(sockFd) {
}

Socket::~Socket() {
}

int Socket::fd() {
    return sockFd_;
}

bool Socket::getTcpInfo(struct tcp_info *tcpInfo) const {
    socklen_t len = sizeof(*tcpInfo);
    memset(tcpInfo, 0, len);
    return ::getsockopt(sockFd_, SOL_TCP, TCP_INFO, tcpInfo, &len) == 0;
}

void Socket::setKeepAlive(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockFd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));
}

bool Socket::getTcpInfoString(std::string &infoString) const {
    struct tcp_info tcpInfo;
    bool            ok = getTcpInfo(&tcpInfo);
    if (ok) {
        std::stringstream ss;
        ss << "unrecovered=" << tcpInfo.tcpi_retransmits << " ";
        ss << "rto=" << tcpInfo.tcpi_rto << " ";
        ss << "ato=" << tcpInfo.tcpi_ato << " ";
        ss << "snd_mss=" << tcpInfo.tcpi_snd_mss << " ";
        ss << "rcv_mss=" << tcpInfo.tcpi_rcv_mss << " ";
        ss << "lost=" << tcpInfo.tcpi_lost << " ";
        ss << "retrans=" << tcpInfo.tcpi_retrans << " ";
        ss << "rtt=" << tcpInfo.tcpi_rtt << " ";
        ss << "rttvar=" << tcpInfo.tcpi_rttvar << " ";
        ss << "sshthresh=" << tcpInfo.tcpi_snd_ssthresh << " ";
        ss << "cwnd=" << tcpInfo.tcpi_snd_cwnd << " ";
        ss << "total_retrans=" << tcpInfo.tcpi_total_retrans;
        infoString = ss.str();
    }
    return ok;
}

void Socket::bindAddress(const InetAddress &addr) {
    sockets::bindOrDie(sockFd_, addr.getSockAddr());
}

void Socket::listen() {
    sockets::listenOrDie(sockFd_);
}

void Socket::setReuseAddr(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
    int opt = on ? 1 : 0;
    int ret = ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
    if (ret < 0 && on) {
        logger.error("set SO_REUSE PORT failed.");
    }
#else
    if (on) {
        logger.info("SO_REUSEPORT is not supported.");
    }
#endif
}

int Socket::accept(InetAddress *remoteAddr) {
    struct sockaddr_in6 addr;
    socklen_t           len;
    bzero(&addr, sizeof addr);
    int connFd = sockets::accept(sockFd_, &addr);
    if (connFd >= 0) {
        remoteAddr->setSockAddrInet6(addr);
    }
    return connFd;
}

void Socket::shutdownWrite() {
    sockets::shutdownWrite(sockFd_);
}
void Socket::setTcpNoDelay(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockFd_, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));
}

bool Socket::connect(const InetAddress &addr, int timeout) {
    if (timeout == 0) {
        return sockets::connect(sockFd_, addr.getSockAddr());
    }

    int oldFlag = fcntl(sockFd_, F_GETFL, 0);
    fcntl(sockFd_, F_SETFL, oldFlag | O_NONBLOCK);

    auto ret = sockets::connect(sockFd_, addr.getSockAddr());
    if (ret == 0) {
        fcntl(sockFd_, F_SETFL, oldFlag);
        return true;
    } else if (errno != EINPROGRESS) {
        logger.error("connect address [%s] error ...", addr.toIpPort());
        return false;
    }

    struct timeval tm {
        .tv_sec = timeout, .tv_usec = 0
    };
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(sockFd_, &writeSet);
    ret = select(sockFd_ + 1, nullptr, &writeSet, nullptr, &tm);
    if (ret < 0) {
        logger.error("connect address [%s] error ...", addr.toIpPort());
        return false;
    } else if (ret == 0) {
        logger.error("connect address [%s] timeout ...", addr.toIpPort());
        return false;
    }

    if (!FD_ISSET(sockFd_, &writeSet)) {
        logger.error("no event on address [%s] timeout ...", addr.toIpPort());
        return false;
    }

    int       error = -1;
    socklen_t len   = sizeof(error);
    if (getsockopt(sockFd_, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        logger.warning("get socket option failed");
        return false;
    }
    if (error != 0) {
        logger.warning("connection failed after select with the error:%d", error);
        return false;
    }

    logger.info("connection ready after select with the socket:%d", sockFd_);
    fcntl(sockFd_, F_SETFL, oldFlag);
    return true;
}

void Socket::close() {
    if (sockFd_ != -1) {
        sockets::close(sockFd_);
        sockFd_ = -1;
    }
}

} // namespace net
} // namespace muduo
