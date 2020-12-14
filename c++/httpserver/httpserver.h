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

#include "logging.h"
namespace http {
namespace utils {

template <class T> std::string toString(const T &val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

enum HttpVersion { HTTP_1_0, HTTP_1_1 };

static std::string _ltrim(const std::string &src, char ch = ' ') {
    std::string           temp = src;
    std::string::iterator p    = std::find_if(temp.begin(), temp.end(), [ &ch ](char c) { return ch != c; });
    temp.erase(temp.begin(), p);
    return temp;
}

static std::string _rtrim(const std::string &src, char ch = ' ') {
    std::string                   temp = src;
    std::string::reverse_iterator p    = find_if(temp.rbegin(), temp.rend(), [ &ch ](char c) { return ch != c; });
    temp.erase(p.base(), temp.end());
    return temp;
}

static std::string trim(const std::string &src, char ch = ' ') {
    return _rtrim(_ltrim(src, ch), ch);
}

static size_t chunkSize(const std::string &strChunkSize) {
    std::string temp;
    for (size_t i = 0; i < strChunkSize.size(); i++) {
        if ((strChunkSize[ i ] >= '0' && strChunkSize[ i ] <= '9') || (strChunkSize[ i ] >= 'A' && strChunkSize[ i ] <= 'F') || (strChunkSize[ i ] >= 'a' && strChunkSize[ i ] <= 'f')) {
            temp.push_back(strChunkSize[ i ]);
        } else {
            break;
        }
    }
    if (temp.empty())
        return -1;
    return std::stoi(temp, nullptr, 16);
}
static std::vector<std::string> split(const std::string &src, char divider) {
    std::vector<std::string> result;
    std::string              temp;
    for (auto &ch : src) {
        if (ch == divider) {
            if(!utils::trim(temp).empty())
                result.push_back(utils::trim(temp));
            temp.clear();
        } else {
            temp.push_back(ch);
        }
    }
    if (!temp.empty())
        result.push_back(utils::trim(temp));
    return result;
}
} // namespace utils
typedef enum { EncodingLength, EncodingChunk, EncodingGzip, EncodingOther } Encoding;

typedef std::vector<std::pair<std::string, std::string>> ResourceMap;
class HttpRequest {
public:
    HttpRequest();
    template <class T> void setHeader(const std::string &key, const T &val) {
        if (key == "Host")
            m_strRequestHost = utils::toString(val);
        else if (key == "Range")
            m_strRangeBytes = utils::toString(val);
        else
            m_vReqestHeader.push_back(std::pair<std::string, std::string>(key, utils::toString(val)));
    }
    void                 setRequestType(const std::string &reqType);
    std::string          getRequestType() const;
    void                 setRequestPath(const std::string &reqPath);
    std::string          getRequestPath() const;
    void                 setHttpVersion(const std::string &httpversion);
    std::string          getHttpVersion() const;
    void                 setPostParams(const std::string &params);
    std::string          getPostParams() const;
    std::string          toStringHeader();
    friend std::ostream &operator<<(std::ostream &os, HttpRequest &obj) {
        os << "> " << obj.m_strRequestType << " " << obj.m_strRequestPath << " " << obj.m_strRequestHttpVersion << CTRL;
        for (auto &item : obj.m_vReqestHeader)
            os << "> " << item.first << ": " << item.second << CTRL;
        if (!obj.m_strRangeBytes.empty())
            os << "Range: " << obj.m_strRangeBytes << CTRL;
        os << "> Host: " << obj.m_strRequestHost << CTRL;
        if (!obj.m_strRequestParams.empty())
            os << obj.m_strRequestParams << CTRL;
        return os;
    }

private:
    ResourceMap m_vReqestHeader;
    std::string m_strRequestType, m_strRequestPath, m_strRequestHttpVersion;
    std::string m_strRequestParams;
    std::string m_strRequestHost;
    std::string m_strRangeBytes;
};

class HttpResponse {
public:
    template <class T> void setHeader(const std::string &key, const T &val) {
        m_vResponseHeader.push_back(std::pair<std::string, std::string>(key, utils::toString(val)));
    }
    void        setStatusMessage(int statusCode, const std::string &HttpVersion, const std::string &message);
    std::string toResponseHeader();
    void        setBodyString(const std::string &m_strBodyString);

private:
    ResourceMap m_vResponseHeader;
    int         m_nstatusCode;
    std::string m_strHttpVersion;
    std::string m_strMessage;
    std::string m_strBodyString;
};

template<typename T>
struct UrlPatternFilter{
public:
    bool operator()(const T&x, const T&y){
        return x.first == y.first;
    }
};
using Func = std::function<void(HttpRequest &, HttpResponse &)>;
struct Key{
public:    
    Key(const std::string &pattern, const std::string &method="GET", bool needval=false)
        :pattern(pattern), method(method),needval(needval){
        if(needval){
            auto itemVector = utils::split(pattern, '/');
            int i = 0;
            for(auto &item: itemVector){
                if(item.front() == '{' && item.back() == '}'){
                    keyPoint.push_back(i);
                    keySet.push_back(item.substr(1, item.size() - 2));
                }
                i++;
            }
        }
    }
    bool MatchFilter(const std::string &reqPath, std::map<std::string, std::string>&valMap){
        if(!needval) return reqPath == pattern;
        else{
            auto itemList = utils::split(reqPath, '/');    
            for(auto i = 0, j = 0; i < keyPoint.size(); i++){
                int pos = keyPoint[i];
                if(pos >= itemList.size())
                    break;
                valMap.insert(std::pair<std::string, std::string>(keySet[j++],itemList[pos]));
            }
            return true;
        }
    }
public:
    std::string pattern;
    std::string method;
    bool needval;
    std::vector<std::string> keySet;
    std::vector<int> keyPoint;
};

class RequestMapper{
public:
    void insert(const Key &key, http::Func &&F){
        m_RequestMapper.push_back(std::pair<Key, http::Func>(key, F));
    }
    http::Func find(const std::string &RequestPath, std::map<std::string, std::string> &resultMap){
        for(auto& iter: m_RequestMapper){
            if(iter.first.MatchFilter(RequestPath, resultMap)){
                return iter.second;
            }
        }
        return nullptr;
    }
protected:
    std::vector<std::pair<Key, http::Func>> m_RequestMapper;
};

typedef std::map<std::string, Func> RequestMapping;
class ClientThread {
public:
    ClientThread(int serverFd, int clientFd);
    void    handRequest(const RequestMapper &handerMapping);
    void    parseHeader(HttpRequest &request);
    int     recvData(int fd, void *buf, size_t n, int ops);
    int     writeData(int fd, void *buf, size_t n, int ops);
    ssize_t writeResponse(HttpResponse &response);

private:
    void ParseHeaderLine(const std::string &line, std::string &key, std::string &val);
    void ParseFirstLine(const std::string &line, std::string &HttpVersion, std::string &RequestPath, std::string &RequestType);
    bool handleServerResource(HttpRequest &request, HttpResponse &response, const std::string &strServerRoot);
private:
    int m_nThreadserverFd;
    int m_nThreadClientFd;
};

#define METHOD_GETSETTER(Name, type) \
    type m_str##Name; \
    const type get##Name(void) const { return m_str##Name; } \
    void set##Name(const type &val ) { m_str##Name == val; }

class HttpServer {
public:
    class HttpConfig{
    public:
        bool loadConfig(const std::string &strConfigFilePath);
    public:
        METHOD_GETSETTER(ServerRoot, std::string);
    };
public:
    HttpServer(const std::string &strServerName, const std::string &strServerIP = "127.0.0.1", const std::string &strServerDescription = "A simple Http Server", int nPort = 80);
public:
    bool addRequestMapping(const std::string &path, Func&& F);
    bool ExecForever();
    RequestMapper& getMapper() { return m_mapper; }
    bool loadHttpConfig(const std::string &strHttpServerConfig="httpd.conf");
    http::Func doFilter(const std::string &requestPath){
        RequestMapping::iterator iter;
        if((iter=m_mHandleMapping.find(requestPath)) != m_mHandleMapping.end())
            return iter->second;
        return nullptr;
    }
private:
    std::string                           m_strServerName;
    std::string                           m_strServerIP;
    std::string                           m_strServerDescription;
    int                                   m_nPort;
    RequestMapping                        m_mHandleMapping;
    int                                   m_nMaxListenClients;
    int                                   m_nServerFd;
    int                                   m_nEpollTimeOut;
    tlog::logImpl                         m_LogServer;  
    HttpConfig                            m_mConfig;
    RequestMapper                         m_mapper;
};

} // namespace http