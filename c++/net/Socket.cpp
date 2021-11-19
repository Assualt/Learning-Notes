#include "Socket.h"
#include "SocketsOp.h"
#include "base/Logging.h"
#include "net/InetAddress.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
namespace muduo {
namespace net {

Socket::Socket(int sockfd)
    : sockfd_(sockfd) {
}

Socket::~Socket() {
}

int Socket::fd() {
    return sockfd_;
}

bool Socket::getTcpInfo(struct tcp_info *tcpinfo) const {
    socklen_t len = sizeof(*tcpinfo);
    memset(tcpinfo, 0, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpinfo, &len) == 0;
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

bool Socket::getTcpInfoString(std::string &infostring) const {
    struct tcp_info tcpinfo;
    bool            ok = getTcpInfo(&tcpinfo);
    if (ok) {
        std::stringstream ss;
        ss << "unrecovered=" << tcpinfo.tcpi_retransmits << " ";
        ss << "rto=" << tcpinfo.tcpi_rto << " ";
        ss << "ato=" << tcpinfo.tcpi_ato << " ";
        ss << "snd_mss=" << tcpinfo.tcpi_snd_mss << " ";
        ss << "rcv_mss=" << tcpinfo.tcpi_rcv_mss << " ";
        ss << "lost=" << tcpinfo.tcpi_lost << " ";
        ss << "retrans=" << tcpinfo.tcpi_retrans << " ";
        ss << "rtt=" << tcpinfo.tcpi_rtt << " ";
        ss << "rttvar=" << tcpinfo.tcpi_rttvar << " ";
        ss << "sshthresh=" << tcpinfo.tcpi_snd_ssthresh << " ";
        ss << "cwnd=" << tcpinfo.tcpi_snd_cwnd << " ";
        ss << "total_retrans=" << tcpinfo.tcpi_total_retrans;
        infostring = ss.str();
    }
    return ok;
}

void Socket::bindAddress(const InetAddress &addr) {
    sockets::bindOrDie(sockfd_, addr.getSockAddr());
}

void Socket::listen() {
    sockets::listenOrDie(sockfd_);
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret    = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        logger.error("SO_REUSEPORT failed.");
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
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        remoteAddr->setSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::shutdownWrite() {
    sockets::shutdownWrite(sockfd_);
}

} // namespace net
} // namespace muduo
