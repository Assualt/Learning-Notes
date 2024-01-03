//
// Created by 侯鑫 on 2024/1/2.
//

#include "socket_typedef.h"
#include "base/log.h"
#include "socket_ops.h"
#include <sys/fcntl.h>

using namespace ssp::base;
using namespace ssp::net;

namespace {
int MyPrintSSLError(const char* msg, size_t len, void *args)
{
    logger.Error("[SSL-%u]%s", (const char*)args, std::string(msg, len));
    return 0;
}
}

void NormalSocket::Init(int32_t fd, void *args)
{
    fd_ = fd;
#if SUPPORT_OPENSSL
    sslConn_ = std::make_unique<SSL_Connection>();
    if (args != nullptr) {
        sslConn_->handle_ = reinterpret_cast<SSL *>(args);
    }
    InitSSL();
#endif
}

void NormalSocket::Close()
{
#if SUPPORT_OPENSSL
    if (sslConn_ == nullptr) {
        return;
    }
    if (sslConn_->handle_) {
        SSL_free(sslConn_->handle_);
        sslConn_->handle_ = nullptr;
    }
    if (sslConn_->context_) {
        SSL_CTX_free(sslConn_->context_);
        sslConn_->context_ = nullptr;
    }
#endif
}

void NormalSocket::SwitchSSL(bool isClient, const std::string &host)
{
#if SUPPORT_OPENSSL
    if (isClient) {
        sslConn_->context_ = SSL_CTX_new(TLS_client_method());
    } else {
        sslConn_->context_ = SSL_CTX_new(TLS_method());
        SSL_CTX_set_options(sslConn_->context_, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
            SSL_OP_NO_COMPRESSION | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
    }

    if (sslConn_->context_ == nullptr) {
        ERR_print_errors_cb(MyPrintSSLError, (void *) "SSL_CTX_new");
        return Close();
    }

    if (!isClient) {
        logger.Info("switch to ssl server ok");
        return;
    }

    // Create an SSL struct for the connection
    sslConn_->handle_ = SSL_new(sslConn_->context_);
    if (sslConn_->handle_ == nullptr) {
        ERR_print_errors_cb(MyPrintSSLError, (void *) "SSL_New");
        return Close();
    }

    // Connect the SSL struct to our connection
    if (!SSL_set_fd(sslConn_->handle_, fd_)) {
        ERR_print_errors_cb(MyPrintSSLError, (void *) "SSL_set_fd");
        return Close();
    }

    // 对于某些拥有超过1个主机名的web服务器，客户端必须告诉服务器客户端试图连接的确切主机名，这样web服务器才可以提供正确的SSL证书
    // resolve problem sslv3 Alert Handshake Failure
    if (!host.empty()) {
        (void)SSL_set_tlsext_host_name(sslConn_->handle_, host.c_str());
        logger.Debug("set tlsext host name is %s", host);
    }

    // Initiate SSL handshake
    if (SSL_connect(sslConn_->handle_) != 1) {
        ERR_print_errors_cb(MyPrintSSLError, (void *) "SSL_Connect");
        return Close();
    }

    logger.Info("switch to ssl client success");
#endif
}

bool NormalSocket::InitSSL()
{
#if SUPPORT_OPENSSL
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
#endif
    return true;
}

bool NormalSocket::Connect(const InetAddress &address, bool useSsl, std::chrono::seconds timeout)
{
    if (timeout.count() == 0) {
        return sockets::Connect(fd_, address.GetSockAddr());
    }

    return ConnectWithNonBlock(address, useSsl, timeout);
}

bool NormalSocket::ConnectWithNonBlock(const InetAddress &address, bool useSsl, std::chrono::seconds timeout)
{
    int oldFlag = fcntl(fd_, F_GETFL, 0);
    (void)fcntl(fd_, F_SETFL, oldFlag | O_NONBLOCK);

    auto ret = sockets::Connect(fd_, address.GetSockAddr());
    if (ret == 0) {
        (void)fcntl(fd_, F_SETFL, oldFlag);
        return true;
    } else if (errno != EINPROGRESS) {
        logger.Error("connect address [%s] error ... timeout:%d", address.ToIpPort(), timeout.count());
        return false;
    }

    if (!SelectConnecting(address, timeout)) {
        return false;
    }

    logger.Info("connection ready after select with the socket:%d", fd_);
    (void)fcntl(fd_, F_SETFL, oldFlag);
    if (useSsl) {
        SwitchSSL(true, address.Host());
    }
    return true;
}

bool NormalSocket::SelectConnecting(const InetAddress &address, std::chrono::seconds timeout) const
{
    struct timeval tm = {.tv_sec = timeout.count()};
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(fd_, &writeSet);
    auto ret = select(fd_ + 1, nullptr, &writeSet, nullptr, &tm);
    if (ret < 0) {
        log_sys.Error("connect address [%s] error ...", address.ToIpPort());
        return false;
    } else if (ret == 0) {
        log_sys.Error("connect address [%s] timeout ...", address.ToIpPort());
        return false;
    }

    if (!FD_ISSET(fd_, &writeSet)) {
        log_sys.Error("no event on address [%s] timeout ...", address.ToIpPort());
        return false;
    }

    int error = -1;
    socklen_t len   = sizeof(int);
    if (getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        logger.Warning("get socket option failed");
        return false;
    }

    if (error != 0) {
        logger.Warning("connection failed after select with the error:%d", error);
        return false;
    }

    return true;
}

