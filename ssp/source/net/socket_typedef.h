//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_SOCKET_TYPEDEF_H
#define SSP_TOOLS_SOCKET_TYPEDEF_H

#include <cstdint>
#include <memory>
#include <string>

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

    bool Connect();

private:
    static bool InitSSL();

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
