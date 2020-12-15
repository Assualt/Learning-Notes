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

#include "logging.h"
// #include "httpconfig.h"
#include "httputils.hpp"
#include "httprequest.h"
#include "httpresponse.h"

namespace http {
typedef enum { EncodingLength, EncodingChunk, EncodingGzip, EncodingOther } Encoding;
using Func = std::function<bool(HttpRequest &, HttpResponse &)>;
class RequestMapper {
public:
    struct Key {
    public:
        Key(const std::string &pattern, const std::string &method = "GET", bool needval = false);
        bool MatchFilter(const std::string &reqPath, std::map<std::string, std::string> &valMap);
        bool MatchFilter(const std::string &reqPath);

    public:
        std::string pattern;
        std::string method;
        bool needval;
        std::vector<std::string> keySet;
        std::vector<int> keyPoint;
    };
    void addRequestMapping(const Key &key, http::Func &&F);
    http::Func find(const std::string &RequestPath, std::map<std::string, std::string> &resultMap);
    http::Func find(const std::string &RequestPath);

protected:
    std::vector<std::pair<Key, http::Func>> m_vRequestMapper;
};

typedef std::map<std::string, Func> RequestMapping;
class ClientThread {
public:
    ClientThread(int serverFd, int clientFd);
    void handRequest(RequestMapper *handerMapping, HttpConfig *config, const char *strRemoteIP);
    void parseHeader(HttpRequest &request);
    int recvData(int fd, void *buf, size_t n, int ops);
    int writeData(int fd, void *buf, size_t n, int ops);
    ssize_t writeResponse(HttpResponse &response);

private:
    void ParseHeaderLine(const std::string &line, std::string &key, std::string &val);
    void ParseFirstLine(const std::string &line, std::string &HttpVersion, std::string &RequestPath, std::string &RequestType);
    bool handleServerResource(HttpRequest &request, HttpResponse &response, const std::string &strServerRoot);

private:
    int m_nThreadserverFd;
    int m_nThreadClientFd;
};

class HttpServer {
public:
public:
    HttpServer(const std::string &strServerName, const std::string &strServerIP = "127.0.0.1", const std::string &strServerDescription = "A simple Http Server", int nPort = 80);

public:
    bool addRequestMapping(const std::string &path, Func &&F);
    bool ExecForever();
    RequestMapper &getMapper() {
        return m_mapper;
    }
    bool loadHttpConfig(const std::string &strHttpServerConfig = "httpd.conf");

    std::string getServerRoot() { return "./html"; }
public:
    static tlog::logImpl ServerLog;

private:
    std::string m_strServerName;
    std::string m_strServerIP;
    std::string m_strServerDescription;
    int m_nPort;
    int m_nMaxListenClients;
    int m_nServerFd;
    int m_nEpollTimeOut;

    HttpConfig m_mConfig;
    RequestMapper m_mapper;
};

}  // namespace http