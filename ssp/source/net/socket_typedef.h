//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_SOCKET_TYPEDEF_H
#define SSP_TOOLS_SOCKET_TYPEDEF_H

#include <cstdint>
#include <memory>
#include <string>
#include <chrono>
#include "net_address.h"

#if SUPPORT_OPENSSL
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

namespace ssp::net {

struct NormalSocket {
public:
    void Close();

    void Init(int32_t fd, void *args);

    void SwitchSSL(bool isClient, const std::string& host = "");

    bool Connect(const InetAddress &address, bool useSsl, std::chrono::seconds timeout);

    [[nodiscard]] int32_t Fd() const { return fd_; }

    [[nodiscard]] bool IsSupportSSL() const;

    void* AcceptWithSSL(int32_t fd);

    int32_t Read(void *buffer, int32_t len);

    int32_t Write(const void *buffer, int32_t length);

private:
    static bool InitSSL();

    bool ConnectWithNonBlock(const InetAddress &address, bool useSsl, std::chrono::seconds timeout);

    [[nodiscard]] bool SelectConnecting(const InetAddress &address, std::chrono::seconds timeout) const;

private:
#if SUPPORT_OPENSSL
    struct SSL_Connection{
        SSL *    handle_{nullptr};
        SSL_CTX *context_{nullptr};
    };
#else
    struct SSL_Connection {};
#endif
    std::unique_ptr<SSL_Connection> sslConn_{nullptr};
    int32_t fd_{-1};
};

}

#endif //SSP_TOOLS_SOCKET_TYPEDEF_H
