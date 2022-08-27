#pragma once
#include <sstream>
#include <string.h>
namespace muduo {
namespace base {

template <class typeStream> class FormatStream : public typeStream {
public:
    FormatStream()
        : typeStream()
        , m_lpszFormat(&g_nEndFlags) {}

    FormatStream &format(const char *lpszFormat) { // 此处不能使用lpszFormat为string类型,否则会出现错误
        flushFormat();
        m_lpszFormat = lpszFormat;
        return outPrefix();
    }
    template <typename typeArg> FormatStream &arg(const typeArg &val) {
        getSelf() << val;
        return outPrefix();
    }
    void flushFormat() {
        if (*m_lpszFormat) {
            getSelf() << m_lpszFormat;
            m_lpszFormat = &g_nEndFlags;
        }
    }

protected:
    FormatStream &outLoop() {
        while (*m_lpszFormat == '%') {
            const char *lpPos = strchr(m_lpszFormat + 1, '%');
            if (lpPos != nullptr) {
                typeStream::write(m_lpszFormat, static_cast<std::streamsize>(lpPos - m_lpszFormat));
                m_lpszFormat = lpPos + 1;
                if (*m_lpszFormat != '%' || *(m_lpszFormat + 1) == '%')
                    break;
            } else {
                flushFormat();
                break;
            }
        }
        return *this;
    }
    FormatStream &outPrefix() {
        const char *lpPos = strchr(m_lpszFormat, '%');
        if (lpPos != nullptr) {
            typeStream::write(m_lpszFormat, static_cast<std::streamsize>(lpPos - m_lpszFormat));
            m_lpszFormat = lpPos + 1;
            if (*m_lpszFormat == '%' && *(m_lpszFormat + 1) != '%')
                return outLoop();
        } else {
            flushFormat();
        }
        return *this;
    }
    typeStream &getSelf() { return *this; }

protected:
    static char g_nEndFlags;
    const char *m_lpszFormat;
};

template <class typeStream> char FormatStream<typeStream>::g_nEndFlags = '\0';

class FmtString : public FormatStream<std::stringstream> {

public:
    FmtString(const std::string &format) { FormatStream::format(format.c_str()); }
};

} // namespace base
} // namespace muduo
