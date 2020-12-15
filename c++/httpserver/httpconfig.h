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

namespace http {
class HttpConfig {
public:
    bool loadConfig(const std::string &strConfigFilePath) {
        return true;
    }

public:
    std::string getServerRoot() const {
        return m_strServerRoot;
    }
    void setServerRoot(const std::string &strServerRoot) {
        m_strServerRoot = strServerRoot;
    }

private:
    std::string m_strServerRoot;
};

class MyStringBuffer : public std::stringbuf {
public:
    void seekReadPos(ssize_t nPos) {
        std::stringbuf::seekpos(nPos, std::ios_base::in);
    }
    std::string toString() {
        std::string temp;
        seekReadPos(0);
        do {
            temp.push_back(this->sgetc());
        } while (this->snextc() != EOF);
        seekReadPos(0);
        return temp;
    }
    void seekWritePos(ssize_t nPos) {
        std::stringbuf::seekpos(nPos, std::ios_base::out);
    }
    void clear() {
        std::stringbuf::setbuf((char *)"", std::streamsize(0));
    }

    size_t size() {
        seekReadPos(0);
        size_t ret = 0;
        do {
            ret += 1;
        } while (this->snextc() != EOF);
        return ret;
    }
};

} // namespace http