#pragma once
#include "base/nonecopyable.h"
#include <iomanip>
#include <iostream>
#include <memory>
#include <netinet/tcp.h>
#include <sstream>

#ifdef USE_SSL
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

using muduo::base::nonecopyable;

namespace muduo {
namespace net {
class InetAddress;
class Buffer;
class Socket : nonecopyable {
public:
    explicit Socket(int sockFd, void *arg = nullptr);
    ~Socket();

public:
    int fd();

    bool getTcpInfo(struct tcp_info *) const;

    bool getTcpInfoString(std::string &infoString) const;

    void bindAddress(const InetAddress &addr);

    void listen();

    bool connect(const InetAddress &addr, int timeout);

    std::pair<int, void *> accept(InetAddress *remoteAddress);

    void close();

    int32_t write(void *buf, size_t size);

    int32_t read(Buffer &buf);

    int32_t setReadTimeout(int seconds);

    int32_t setWriteTimeout(int seconds);

    ///
    /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    ///
    void setTcpNoDelay(bool on);

    ///
    /// Enable/disable SO_REUSEADDR
    ///
    void setReuseAddr(bool on);

    ///
    /// Enable/disable SO_REUSEPORT
    ///
    void setReusePort(bool on);

    ///
    /// Enable/disable SO_KEEPALIVE
    ///
    void setKeepAlive(bool on);

    void shutdownWrite();

    bool switchToSSL(bool isClient = true, const std::string &host = "");

    bool initSSL();

    bool initSSLServer(const std::string &certPath, const std::string &keyPath, const std::string &passWd = "");

    void sslDisConnect();

    void showTlsInfo();

    void PrintSSLError(const std::string &funcName, int32_t ret);

private:
    int sockFd_{-1};

#ifdef USE_SSL
    typedef struct {
        SSL     *m_ptrHandle;
        SSL_CTX *m_ptrContext;
    } SSL_Connection;
    std::unique_ptr<SSL_Connection> sslConn_{nullptr};
#endif
};
} // namespace net
} // namespace muduo