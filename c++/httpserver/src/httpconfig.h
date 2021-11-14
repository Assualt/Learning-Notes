#pragma once
#include "configure.h"
#include <iostream>
#include <sstream>
#define PARAM_SETER_GETER(strParamName, valType)          \
public:                                                   \
    inline const valType &get##strParamName(void) const { \
        return m_##strParamName;                          \
    }                                                     \
    inline void set##strParamName(const valType &n) {     \
        m_##strParamName = n;                             \
    }                                                     \
    inline void set##strParamName(valType &&n) {          \
        m_##strParamName = std::move(n);                  \
    }                                                     \
                                                          \
protected:                                                \
    valType m_##strParamName

#define CTRL "\r\n"
#define ContentType "Content-Type"
#define ContentEncoding "Content-Encoding"
#define Accept "Accept"
#define AcceptEncoding "Accept-Encoding"
#define Authorization "Authorization"
#define AcceptLanguage "Accept-Language"
#define UserAgent "User-Agent"
#define ContentLength "Content-Length"
#define TransferEncoding "Transfer-Encoding"
#define AcceptRanges "Accept-Ranges"
#define Location "Location"
#define Cookie "Cookie"
#define Referer "Referer"

#define NOTFOUNDHTML "<html><head><title>404 Not Found</title></head><body>404 not found</body></html>"
#define NOTFOUND "/404"

#define SERVER "Server"
#define SERVERVal "HttpServer/0.1 Linux/GNU gcc/c++"

#define AUTHREQUIRED                                                                                                                         \
    "<html>\r\n<head>\r\n<title>401 Authorization Required</title>\r\n</head>\r\n<body bgcolor=\"white\">\r\n<center><h1>401 Authorization " \
    "Required</h1></center>\r\n<hr>\r\n<center>httpserver</center>\r\n</body>\r\n</html>\r\n"

#define BADREQUEST                                                                                                      \
    "<html>\r\n<head>\r\n<title>401 Bad Request</title>\r\n</head>\r\n<body bgcolor=\"white\">\r\n<center><h1>401 Bad " \
    "Request</h1></center>\r\n<hr>\r\n<center>httpserver</center>\r\n</body>\r\n</html>"

#define METHODNOTALLOWED                                                                                                                       \
    "<html>\r\n<head>\r\n<title>405 Method not Allowed</title>\r\n</head>\r\n<body bgcolor=\"white\">\r\n<center><h1>405 Method  not Allowed." \
    "</h1></center>\r\n<hr>\r\n<center>httpserver</center>\r\n</body>\r\n</html>"

#include "hashutils.hpp"
#include "httputils.h"
#include "logging.h"
#include <set>
#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#endif
namespace http {

enum SSLProtocols { Type_TLS1_1, Type_TLS1_2, Type_TLS1_3, Type_TLS1_23 };

class ConnectionInfo {
public:
    std::string m_strConnectIP;
    std::string m_strServerRoot;
    int         m_nPort;
    int         m_nClientFd;
    int         m_nFDFlag;
    bool        m_bClosed;
#ifdef USE_OPENSSL
    SSL *ssl;
#endif
    ConnectionInfo()
        : m_nClientFd(-1) {
        m_bClosed = false;
#ifdef USE_OPENSSL
        ssl = nullptr;
#endif
    }
    void disconnect() {
#ifdef USE_OPENSSL
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = nullptr;
        }
#endif
        close(m_nClientFd);
        m_bClosed = true;
    }
};

class SSLConfig {
public:
    SSLProtocols protocol;
    std::string  servername;
    std::string  certificate;
    std::string  certificate_private_key;
    std::string  ciphers;
    std::string  ca;
};

struct StringCaseCmp : std::binary_function<std::string, std::string, bool> {
public:
    int operator()(const string &lhs, const string &rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

class HttpConfig {
public:
    HttpConfig();
    ~HttpConfig();
    bool        loadConfig(const std::string &strConfigFilePath);
    bool        loadMimeType(const std::string &mimeType = "mime.types");
    std::string getMimeType(const std::string &strFileName);

public:
    std::string getServerRoot() const {
        return m_strServerRoot;
    }
    std::string getServerHost() const {
        return m_strServerHost;
    }
    int getServerPort() const {
        return m_nServerPort;
    }
    int getMaxAcceptClient() const {
        return m_nMaxAcceptClients;
    }
    std::string getAuthName() const {
        return m_strAuthName;
    }
    SSLConfig getSSLConfig() const {
        return m_sslconfig;
    }
    std::string getLoggerFormat() const {
        return m_strLoggerFmt;
    }
    bool supportSSL() const {
        return m_bSSLServer;
    }
    void                                 loadDirentTmplateHtml(const std::string &tmplatePath);
    std::string &                        getDirentTmplateHtml();
    std::set<std::string, StringCaseCmp> getSuffixSet();
    bool                                 needAuth();
    bool                                 checkAuth(const std::string &AuthString);
    bool                                 checkMethod(const std::string &RequestMethod);
    bool                                 checkHttpVersion(const std::string &HttpVersion);

protected:
    void loadAuthFile(const std::string &strAuthFile);
    void parseSection(const std::string strSectionName, const std::string &strSection);
    void initSetting();

private:
    bool        m_bSSLServer;
    std::string m_strServerRoot;
    int         m_nServerPort;
    std::string m_strServerHost;
    int         m_nMaxAcceptClients;
    std::string m_strAuthName;
    SSLConfig   m_sslconfig;

    std::string                                               m_strDirentTmplateHtml;
    std::set<std::string, StringCaseCmp>                      m_SuffixSet;
    std::map<std::string, std::string>                        m_ExtMimeType;
    bool                                                      m_bRequiredAuth;
    std::map<std::string, std::string>                        m_AuthPassMap;
    std::map<std::string, std::map<std::string, std::string>> m_SectionMap;
    std::set<std::string, StringCaseCmp>                      m_SupportMethodSet;
    std::set<std::string, StringCaseCmp>                      m_SupportHttpVersionSet;
    std::string                                               m_strLoggerFmt;
    conf::ConfigureManager *                                  m_ptrConfigure;
};

} // namespace http