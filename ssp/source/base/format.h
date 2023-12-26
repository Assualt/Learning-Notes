//
// Created by 侯鑫 on 2023/12/26.
//

#ifndef SSP_TOOLS_FORMAT_H
#define SSP_TOOLS_FORMAT_H

#include <cstring>
#include <iomanip>
#include <sstream>

namespace ssp::base {
template <class typeStream> class FormatStream : public typeStream {
public:
    FormatStream()
        : typeStream()
    {
    }

    FormatStream &format(const char *format) // 此处不能使用format为string类型,否则会出现错误
    {
        flushFormat();
        format_ = format;
        return outPrefix();
    }

    template <typename typeArg> FormatStream &arg(const typeArg &val)
    {
        getSelf() << val;
        return outPrefix();
    }

    void flushFormat()
    {
        if (*format_) {
            getSelf() << format_;
            format_ = &g_nEndFlags;
        }
    }

protected:
    FormatStream &outLoop()
    {
        while (*format_ == '%') {
            const char *lpPos = strchr(format_ + 1, '%');
            if (lpPos != nullptr) {
                typeStream::write(format_, static_cast<std::streamsize>(lpPos - format_));
                format_ = lpPos + 1;
                if (*format_ != '%' || *(format_ + 1) == '%')
                    break;
            } else {
                flushFormat();
                break;
            }
        }
        return *this;
    }

    FormatStream &outPrefix()
    {
        const char *lpPos = strchr(format_, '%');
        if (lpPos != nullptr) {
            typeStream::write(format_, static_cast<std::streamsize>(lpPos - format_));
            format_ = lpPos + 1;
            if (*format_ == '%' && *(format_ + 1) != '%')
                return outLoop();
        } else {
            flushFormat();
        }
        return *this;
    }

    typeStream &getSelf() { return *this; }

protected:
    [[maybe_unused]] static char g_nEndFlags;
    const char *format_ {nullptr };
};

template <class typeStream> char FormatStream<typeStream>::g_nEndFlags = '\0';

class FmtString : public FormatStream<std::stringstream> {
public:
    explicit FmtString(const std::string &format) { FormatStream::format(format.c_str()); }
};

template <class T> std::string ToFixedString(T val, int width, bool isLeft, char ch = '0') {
    std::stringstream ss;
    ss << std::setw(width) << (isLeft ? std::left : std::right) << std::setfill(ch) << val;
    return ss.str();
}
} // namespace ssp::base

#endif //SSP_TOOLS_FORMAT_H
