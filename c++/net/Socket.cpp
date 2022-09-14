#include "Socket.h"
#include "SocketsOp.h"
#include "base/Logging.h"
#include "net/Buffer.h"
#include "net/InetAddress.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
namespace muduo {
namespace net {

Socket::Socket(int sockFd, void *arg)
    : sockFd_(sockFd) {
    if (arg != nullptr) {
        sslConn_              = std::make_unique<SSL_Connection>();
        sslConn_->m_ptrHandle = reinterpret_cast<SSL *>(arg);
    }
}

Socket::~Socket() {}

int Socket::fd() { return sockFd_; }

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
        ss << "unRecovered=" << tcpInfo.tcpi_retransmits << " ";
        ss << "rto=" << tcpInfo.tcpi_rto << " ";
        ss << "ato=" << tcpInfo.tcpi_ato << " ";
        ss << "snd_mss=" << tcpInfo.tcpi_snd_mss << " ";
        ss << "rcv_mss=" << tcpInfo.tcpi_rcv_mss << " ";
        ss << "lost=" << tcpInfo.tcpi_lost << " ";
        ss << "reTrans=" << tcpInfo.tcpi_retrans << " ";
        ss << "rtt=" << tcpInfo.tcpi_rtt << " ";
        ss << "rttVar=" << tcpInfo.tcpi_rttvar << " ";
        ss << "sshThresh=" << tcpInfo.tcpi_snd_ssthresh << " ";
        ss << "cwnd=" << tcpInfo.tcpi_snd_cwnd << " ";
        ss << "total_reTrans=" << tcpInfo.tcpi_total_retrans;
        infoString = ss.str();
    }
    return ok;
}

void Socket::bindAddress(const InetAddress &addr) { sockets::bindOrDie(sockFd_, addr.getSockAddr()); }

void Socket::listen() { sockets::listenOrDie(sockFd_); }

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
    LOG_IF(on).info("SO_REUSEPORT is not supported.");
#endif
}

std::pair<int, void *> Socket::accept(InetAddress *remoteAddr) {
    struct sockaddr_in6 addr;
    socklen_t           len;
    bzero(&addr, sizeof addr);
    int connFd = sockets::accept(sockFd_, &addr);
    if (connFd >= 0) {
        remoteAddr->setSockAddrInet6(addr);
    }

#ifndef USE_SSL
    return {connFd, nullptr};
#else
    if (sslConn_ == nullptr) {
        return {connFd, nullptr};
    }

    SSL *ssl = nullptr;
    ssl      = SSL_new(sslConn_->m_ptrContext);
    if (SSL_set_fd(ssl, connFd) != 1) {
        ERR_print_errors_fp(stderr);
        sockets::close(connFd);
        logger.info("tls/ssl set fd fail.");
        return {-1, nullptr};
    }

    auto ret = SSL_accept(ssl);
    if (ret != 1) {
        ERR_print_errors_fp(stderr);
        logger.info("tls/ssl handle shake fail. ret:%d", SSL_get_error(ssl, ret));
        sockets::close(connFd);
        return {-1, nullptr};
    }
    return {connFd, ssl};
#endif
}

void Socket::shutdownWrite() { sockets::shutdownWrite(sockFd_); }

void Socket::setTcpNoDelay(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(sockFd_, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));
}

bool Socket::connect(const InetAddress &addr, int timeout) {
    if (timeout <= 0) {
        return sockets::connect(sockFd_, addr.getSockAddr());
    }

    int oldFlag = fcntl(sockFd_, F_GETFL, 0);
    (void)fcntl(sockFd_, F_SETFL, oldFlag | O_NONBLOCK);

    auto ret = sockets::connect(sockFd_, addr.getSockAddr());
    if (ret == 0) {
        (void)fcntl(sockFd_, F_SETFL, oldFlag);
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
    (void)fcntl(sockFd_, F_SETFL, oldFlag);
    return true;
}

uint32_t Socket::write(void *buf, size_t size) {
    if (sockFd_ == -1) {
        return -1;
    }

    if (sslConn_ == nullptr) {
        return sockets::write(sockFd_, buf, size);
    }

#ifdef USE_SSL
    return SSL_write(sslConn_->m_ptrHandle, buf, size);
#else
    return sockets::write(sockFd_, buf, size);
#endif
}

uint32_t Socket::read(Buffer &buf) {
    if (sockFd_ == -1) {
        return -1;
    }

    char   buffer[ 8192 ] = {0};
    int    nRead;
    size_t nTotal = 0;
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        if (sslConn_ == nullptr) {
            nRead = sockets::read(sockFd_, buffer, sizeof(buffer));
        } else {
#ifdef USE_SSL
            if (sslConn_ != nullptr && sslConn_->m_ptrHandle != nullptr) {
                nRead = SSL_read(sslConn_->m_ptrHandle, buffer, sizeof(buffer));
            } else {
                nRead = sockets::read(sockFd_, buffer, sizeof(buffer));
            }
#else
            nRead = sockets::read(sockFd_, buffer, sizeof(buffer));
#endif
        }
        if (nRead <= 0) {
            break;
        }

        buf.append(buffer, nRead);
        nTotal += nRead;
#if 0
        printf("\n Read ======> %d\n", nRead);
        for (auto idx = 0; idx < nRead; idx++) {
            printf("%02x ", buffer[ idx ] & 0xFF);
            if (buffer[ idx ] == '\n' && idx > 1 && buffer[ idx - 1 ] == 0xd) {
                printf("\n");
            }
        }
#endif

        if (nRead < sizeof(buffer)) {
            break;
        }
    }

    return nTotal;
}

bool Socket::initSSL() {
#ifdef USE_SSL
#if OPENSSL_VERSION_NUMBER < 0x1010001fL
    OPENSSL_config(nullptr);
    // Register the error strings for libcrypto & libssl
    SSL_load_error_strings();

    // Register the available ciphers and digests
    SSL_library_init();

    OpenSSL_add_all_algorithms();
    return true;
#else
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CONFIG | OPENSSL_INIT_LOAD_CRYPTO_STRINGS,
                     nullptr);
    ERR_clear_error();
#endif
    return true;
#endif
}

void Socket::sslDisConnect() {
#ifdef USE_SSL
    if (sslConn_ == nullptr) {
        return;
    }
    if (sslConn_->m_ptrHandle) {
        SSL_shutdown(sslConn_->m_ptrHandle);
        sslConn_->m_ptrHandle = nullptr;
    }
    if (sslConn_->m_ptrContext) {
        SSL_CTX_free(sslConn_->m_ptrContext);
        sslConn_->m_ptrContext = nullptr;
    }
#endif
}

bool Socket::switchToSSL(bool isClient) {
#ifdef USE_SSL
    sslConn_ = std::make_unique<SSL_Connection>();
    if (isClient) {
        sslConn_->m_ptrContext = SSL_CTX_new((TLS_client_method()));
    } else {
        sslConn_->m_ptrContext = SSL_CTX_new(TLS_method());
        SSL_CTX_set_options(sslConn_->m_ptrContext, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
                                                        SSL_OP_NO_COMPRESSION |
                                                        SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
    }
    if (sslConn_->m_ptrContext == nullptr) {
        sslDisConnect();
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (!isClient) {
        logger.info("switch to ssl server ok");
        return true;
    }

    // Create an SSL struct for the connection
    sslConn_->m_ptrHandle = SSL_new(sslConn_->m_ptrContext);
    if (sslConn_->m_ptrHandle == nullptr) {
        sslDisConnect();
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Connect the SSL struct to our connection
    if (!SSL_set_fd(sslConn_->m_ptrHandle, sockFd_)) {
        sslDisConnect();
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Initiate SSL handshake
    if (SSL_connect(sslConn_->m_ptrHandle) != 1) {
        ERR_print_errors_fp(stderr);
        sslDisConnect();
        return false;
    }

    logger.info("switch to ssl success");
    return true;
#else
    return false;
#endif
}

void Socket::close() {
    if (sockFd_ != -1) {
        sockets::close(sockFd_);
        sockFd_ = -1;
    }
}

int32_t Socket::setReadTimeout(int seconds) {
    struct timeval val {
        .tv_sec = seconds, .tv_usec = 0
    };

    return setsockopt(sockFd_, SOL_SOCKET, SO_RCVTIMEO, (const void *)&val, sizeof(val));
};

int32_t Socket::setWriteTimeout(int seconds) {
    struct timeval val {
        .tv_sec = seconds, .tv_usec = 0
    };

    return setsockopt(sockFd_, SOL_SOCKET, SO_SNDTIMEO, (const void *)&val, sizeof(val));
}

bool Socket::initSSLServer(const std::string &certPath, const std::string &keyPath, const std::string &passWd) {
#ifndef USE_SSL
    return false;
#else
    auto ret = initSSL();
    if (!ret) {
        logger.info("init ssl error");
        return false;
    }

    ret = switchToSSL(false);
    if (!ret) {
        ERR_print_errors_fp(stderr);
        sslDisConnect();
        return ret;
    }

    auto code = SSL_CTX_use_certificate_file(sslConn_->m_ptrContext, certPath.c_str(), SSL_FILETYPE_PEM);
    if (code != 1) {
        ERR_print_errors_fp(stderr);
        sslDisConnect();
        return false;
    }

    if (!passWd.empty()) {
        SSL_CTX_set_default_passwd_cb_userdata(sslConn_->m_ptrContext, (void *)passWd.c_str());
    }

    code = SSL_CTX_use_PrivateKey_file(sslConn_->m_ptrContext, keyPath.c_str(), SSL_FILETYPE_PEM);
    if (code != 1) {
        ERR_print_errors_fp(stderr);
        sslDisConnect();
        return false;
    }

    code = SSL_CTX_check_private_key(sslConn_->m_ptrContext);
    return (code == 0);
#endif
}

void Socket::showTlsInfo() {
#ifdef USE_SSL
    if (sslConn_ == nullptr || sslConn_->m_ptrHandle == nullptr) {
        return;
    }

    // 获取数字证书信息
    auto cert = SSL_get_peer_certificate(sslConn_->m_ptrHandle);
    if (cert == nullptr) {
        return;
    }

    auto str = X509_NAME_oneline(X509_get_subject_name(cert), nullptr, 0);
    logger.info("subject:%s", str);

    string startTime = reinterpret_cast<const char *>(X509_get0_notBefore(cert)->data);
    string endTime   = reinterpret_cast<const char *>(X509_get0_notAfter(cert)->data);

    logger.info("startTime:20%s endTime:20%s", startTime, endTime);

    // issuer
    auto issuer = X509_NAME_oneline(X509_get_issuer_name(cert), nullptr, 0);
    logger.info("issuer:%s", issuer);

    X509_free(cert);
#endif
}

} // namespace net
} // namespace muduo
