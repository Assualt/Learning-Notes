#include "base/nonecopyable.h"
#include <iostream>

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

using muduo::base::nonecopyable;
class HttpConfig : nonecopyable {
public:
    HttpConfig() = default;
    HttpConfig(const char *configPath);

    void Init(const std::string &path);

public:
    PARAM_SETER_GETER(AuthName, std::string);
    PARAM_SETER_GETER(ServerRoot, std::string);
    PARAM_SETER_GETER(DirentTmplateHtml, std::string);
};