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

#define AUTHREQUIRED                                                                                                                         \
    "<html>\r\n<head>\r\n<title>401 Authorization Required</title>\r\n</head>\r\n<body bgcolor=\"white\">\r\n<center><h1>401 Authorization " \
    "Required</h1></center>\r\n<hr>\r\n<center>httpserver</center>\r\n</body>\r\n</html>\r\n"

#define BADREQUEST                                                                                                      \
    "<html>\r\n<head>\r\n<title>401 Bad Request</title>\r\n</head>\r\n<body bgcolor=\"white\">\r\n<center><h1>401 Bad " \
    "Request</h1></center>\r\n<hr>\r\n<center>httpserver</center>\r\n</body>\r\n</html>"

#include "hashutils.hpp"
#include "httputils.h"
#include "logging.h"
#include <set>

namespace http {

struct StringCaseCmp : std::binary_function<std::string, std::string, bool> {
public:
    bool operator()(const string &lhs, const string &rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str());
    }
};

class HttpConfig {
public:
    HttpConfig();
    bool        loadConfig(const std::string &strConfigFilePath);
    bool        loadMimeType(const std::string &mimeType = "mime.types");
    std::string getMimeType(const std::string &strFileName);

public:
    std::string getServerRoot() const;
    void        setServerRoot(const std::string &strServerRoot);

    void                                 loadDirentTmplateHtml(const std::string &tmplatePath);
    std::string &                        getDirentTmplateHtml();
    std::set<std::string, StringCaseCmp> getSuffixSet();
    bool                                 needAuth();
    bool                                 checkAuth(const std::string &AuthString);
    bool                                 checkMethod(const std::string &RequestMethod);
    bool                                 checkHttpVersion(const std::string &HttpVersion);
    std::string                          getLoggerFormat() {
        return m_strLoggerFmt;
    }

protected:
    void loadAuthFile(const std::string &strAuthFile);
    void parseSection(const std::string strSectionName, const std::string &strSection);

private:
    std::string                                               m_strServerRoot;
    std::string                                               m_strDirentTmplateHtml;
    std::set<std::string, StringCaseCmp>                      m_SuffixSet;
    std::map<std::string, std::string>                        m_ExtMimeType;
    bool                                                      m_bRequiredAuth;
    std::map<std::string, std::string>                        m_AuthPassMap;
    std::map<std::string, std::map<std::string, std::string>> m_SectionMap;
    std::set<std::string, StringCaseCmp>                      m_SupportMethodSet;
    std::set<std::string, StringCaseCmp>                      m_SupportHttpVersionSet;
    std::string                                               m_strLoggerFmt;
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