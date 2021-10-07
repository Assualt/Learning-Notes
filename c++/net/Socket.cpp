#include <string.h>
#include "Socket.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
namespace muduo {
namespace net{

Socket::Socket(int sockfd) 
    :sockfd_(sockfd_) {
}

Socket::~Socket() {
}

bool Socket::getTcpInfo(struct tcp_info *tcpinfo) const {
    socklen_t len = sizeof(*tcpinfo);
    memset(tcpinfo, 0, len); 
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpinfo, &len) == 0;
}

bool Socket::getTcpInfoString(std::string &infostring) const {
    struct tcp_info tcpinfo;
    bool ok = getTcpInfo(&tcpinfo);
    if(ok) {
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

}

void Socket::listen() {

}

int Socket::accept(InetAddress &RemoteAddr) {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len;
    int ConnectedFd = ::accept(sockfd_, (struct sockaddr *)&addr, &len);
    if(ConnectedFd >0) {
        //
    }
    return ConnectedFd;
}



} // namespace net
} // namespace muduo
