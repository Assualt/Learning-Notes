#pragma once
#include "base/Configure.h"
#include "base/nonecopyable.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <set>

using namespace muduo::base;
#define PARAM_SETTER_GETTER(strParamName, valType)                                                                     \
public:                                                                                                                \
    inline const valType &get##strParamName(void) const { return m_##strParamName; }                                   \
    inline void           set##strParamName(const valType &n) { m_##strParamName = n; }                                \
    inline void           set##strParamName(valType &&n) { m_##strParamName = std::move(n); }                          \
                                                                                                                       \
protected:                                                                                                             \
    valType m_##strParamName;

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

#define PATTERN_METHOD_NOT_ALLOWED "/405"

#define Date "Date"
#define LastModified "Last-Modified"

#define Connection "Connection"

#define SERVER "Server"
#define SERVERVal "HttpServer/0.1 Linux/GNU gcc/c++"

#define WwwAuthenticate "WWW-Authenticate"

// for websocket
#define Upgrade "Upgrade"
#define SecWebSocketVersion "Sec-WebSocket-Version"
#define SecWebSocketKey "Sec-WebSocket-Key"
#define Origin "Origin"
#define SecWebSocketAccept "Sec-WebSocket-Accept"

#define FilePattern "/#*#/"
#define DefaultPattern "/#@#/"

#define CHUNK_SIZE 1024

enum HttpContentType {
    ContentRaw,   /* Content-Type: length */
    ContentStream /* Content-Bytes: bytes*/
};

enum EncodingType {
    Type_Gzip,
    Type_Br,
    Type_Deflate,
    Type_Raw,
};

enum HttpStatusCode : uint32_t {
    kUnknown,
    k101ProtocolSwitch   = 101,
    k200Ok               = 200,
    k301MovedPermanently = 301,
    k302MovedPermanently = 302,
    k400BadRequest       = 400,
    k401NotAuth          = 401,
    k404NotFound         = 404,
    k405MethodNotAllow   = 405,
};

enum HttpVersion : uint32_t {
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2_0,
    HTTP_UNKNOWN,
};

enum REQ_TYPE : uint32_t {
    TYPE_GET  = (0x1 << 0),
    TYPE_POST = (0x1 << 1),
    TYPE_PUT  = (0x1 << 2),
    TYPE_BUT  = UINT32_MAX,
};

inline std::string getHttpVersionString(HttpVersion version) {
    static std::map<HttpVersion, std::string> verMap = {
        {HTTP_1_0, "HTTP/1.0"},
        {HTTP_1_1, "HTTP/1.1"},
        {HTTP_2_0, "HTTP/2.0"},
    };

    auto itr = verMap.find(version);
    if (itr != verMap.end()) {
        return itr->second;
    }

    return verMap[ HTTP_1_1 ];
}

inline HttpVersion getHttpVersionByString(const std::string &ver) {
    static std::map<std::string, HttpVersion> verStrMap = {
        {"HTTP/1.0", HTTP_1_0},
        {"HTTP/1.1", HTTP_1_1},
        {"HTTP/2.0", HTTP_2_0},
    };

    auto itr = verStrMap.find(ver);
    if (itr != verStrMap.end()) {
        return itr->second;
    }

    return HttpVersion::HTTP_1_1;
}

inline bool isValidRequest(const std::string &requestType) {
    static std::set<std::string> reqTypeMap = {
        "GET", "POST", "PUT", "HEAD", "DELETE",
    };

    std::string type = requestType;
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);
    return reqTypeMap.find(type) != reqTypeMap.end();
}

using muduo::base::nonecopyable;
class HttpConfig : nonecopyable {
public:
    HttpConfig() = default;

    HttpConfig(const char *configPath);

    void Init(const std::string &path);

private:
    std::shared_ptr<ConfigureManager> mgr_{nullptr};

public:
    PARAM_SETTER_GETTER(SSLCertPath, std::string);
    PARAM_SETTER_GETTER(SSLPrivateKeyPath, std::string);
    PARAM_SETTER_GETTER(SupportSSL, bool);
    PARAM_SETTER_GETTER(ServerName, std::string);
    PARAM_SETTER_GETTER(AuthName, std::string);
    PARAM_SETTER_GETTER(ServerRoot, std::string);
    PARAM_SETTER_GETTER(ServerPort, int32_t);
    PARAM_SETTER_GETTER(DirentTmplateHtml, std::string);
    PARAM_SETTER_GETTER(RecoredLog, bool);
    PARAM_SETTER_GETTER(SupportWebSocket, bool);
    PARAM_SETTER_GETTER(WebSocketPort, int);
    PARAM_SETTER_GETTER(WebSocketThreadNum, int);
};