#pragma once
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

#include "hashutils.hpp"
#include "httputils.h"
#include "logging.h"
#include <set>

namespace http {
class HttpConfig {
public:
    HttpConfig();
    bool              loadConfig(const std::string &strConfigFilePath);
    bool              loadMimeType(const std::string &mimeType = "mime.types");
    const std::string getMimeType(const std::string &strFileName);

public:
    std::string getServerRoot() const;
    void        setServerRoot(const std::string &strServerRoot);

    void                        loadDirentTmplateHtml(const std::string &tmplatePath);
    std::string &               getDirentTmplateHtml();
    const std::set<std::string> getSuffixSet();

private:
    std::string                        m_strServerRoot;
    std::string                        m_strDirentTmplateHtml;
    std::set<std::string>              m_SuffixSet;
    std::map<std::string, std::string> m_ExtMimeType;
};

class ConnectionInfo {
public:
    std::string m_strConnectIP;
    std::string m_strServerRoot;
    int         m_nPort;
    int         m_nClientFd;
    int         m_nFDFlag;
};

} // namespace http