#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#ifdef USE_OPENSSL
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

// net
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <sys/epoll.h>

#include <errno.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <vector>

#include "httprequest.h"
#include "httpresponse.h"
#include "logging.h"
#include "threadpool.h"
#include <queue>

namespace http {
typedef enum { EncodingLength, EncodingChunk, EncodingGzip, EncodingOther } Encoding;
using Func = std::function<bool(HttpRequest &, HttpResponse &, HttpConfig &)>;
class RequestMapper {
public:
    struct Key {
    public:
        Key(const std::string &pattern, const std::string &method = "GET", bool needval = false);
        bool MatchFilter(const std::string &reqPath, const std::string &reqType, std::map<std::string, std::string> &valMap, bool &MethodAllowed);
        bool MatchFilter(const std::string &reqPath, const std::string &reqType, bool &MethodAllowed);

    public:
        std::string              pattern;
        std::string              method;
        bool                     needval;
        std::vector<std::string> keySet;
        std::vector<int>         keyPoint;
    };
    void       addRequestMapping(const Key &key, http::Func &&F);
    http::Func find(const std::string &RequestPath, const std::string &reqType, std::map<std::string, std::string> &resultMap);
    http::Func find(const std::string &RequestPath, const std::string &reqType);

protected:
    std::vector<std::pair<Key, http::Func>> m_vRequestMapper;
};

typedef std::map<std::string, Func> RequestMapping;
class ClientThread {
public:
    // ClientThread(int serverFd, int clientFd);
    static void    handleRequest(RequestMapper *handlerMapping, HttpConfig *config, ConnectionInfo *info);
    static bool    parseHeader(ConnectionInfo *info, HttpRequest &request, HttpConfig *config);
    static int     recvData(ConnectionInfo *info, void *buf, size_t n, int ops);
    static int     writeData(int fd, void *buf, size_t n, int ops);
    static ssize_t writeResponse(int client_fd, HttpResponse &response);

private:
    static void ParseHeaderLine(const std::string &line, std::string &key, std::string &val);
    static void ParseFirstLine(const std::string &line, std::string &HttpVersion, std::string &RequestPath, std::string &RequestType);
    static bool handleServerResource(HttpRequest &request, HttpResponse &response, const std::string &strServerRoot);
};

class HttpServer {
public:
    HttpServer();

public:
    bool           addRequestMapping(const std::string &path, Func &&F);
    bool           ExecForever();
    RequestMapper &getMapper() {
        return m_mapper;
    }
    bool        loadHttpConfig(const std::string &strHttpServerConfig = "httpd.conf");
    HttpConfig &getHttpConfig() {
        return m_mConfig;
    }
    void StartThreads(int nCount) {
        ThreadsPool.startPool(nCount);
    }
    int setFDnonBlock(int fd);

    std::string getServerRoot() {
        return m_mConfig.getServerRoot();
    }
#ifdef USE_OPENSSL
    bool switchToSSLServer();

private:
    SSL_METHOD *meth;
    SSL_CTX *   ctx;
#endif

private:
    std::string m_strServerName;
    std::string m_strServerIP;
    std::string m_strServerDescription;
    int         m_nPort;
    int         m_nMaxListenClients;
    int         m_nServerFd;
    int         m_nEpollTimeOut;

    HttpConfig      m_mConfig;
    RequestMapper   m_mapper;
    std::threadpool ThreadsPool;
};

} // namespace http