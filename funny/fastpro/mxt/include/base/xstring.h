#ifndef _X_STRING_H_2018_12_14_
#define _X_STRING_H_2018_12_14_

#include <list>
#include "xmtdef.h"
#include "string.h"
#include "xexception.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;
NAMESPACE_BEGIN

typedef std::string tstring;
typedef std::wstring twstring;
typedef std::istream tistream;
typedef std::ostream tostream;
typedef std::ifstream tifstream;
typedef std::ofstream tofstream;
typedef std::ostringstream tostrstream;
inline void _ltrim(tstring &src, const char ch) {
    size_t _pos = 0;
    while (src[_pos++] == ch)
        ;
    src.erase(0, _pos);
}
inline void _rtrim(tstring &src, const char ch) {
    size_t _pos = src.length() - 1;
    while (src[_pos--] == ch)
        ;
    src.erase(_pos);
}
inline void _trim(tstring &src, const char ch) {
    _ltrim(src, ch);
    _rtrim(src, ch);
}
template <class val_type>
extern tstring toString(val_type val) {
    std::stringstream buffer;
    buffer << val;
    return buffer.str();
}
class TStringHelper {
public:
    static tstring m_strEmptyString;
    static void split(std::list<tstring> &ret, const tstring &_src, const char _del = ',');
    static void splitBytes(std::list<tstring> &ret, const tstring &_src, size_t len = 8);
    static tstring ltrim(tstring src);    //去掉左方空格
    static tstring trim(tstring src);     //去掉左右方空格
    static tstring rtrim(tstring src);    //去掉右方空格
    static char toupper(char c);          //大写
    static char tolower(char c);          //小写
    static tstring tolower(tstring src);  //小写
    static tstring toupper(tstring src);  //大写
    static tstring replaceAll(tstring src, const char _src,
                              const char _des);  //字符替换
    static tstring replaceAll(tstring src, const char *_src,
                              const char *_des);  //字符串替换，建议
    static bool startWith(const tstring &src, const char *prefix);
    static bool endWith(const tstring &src, const char *backfix);
    static int toInt(const tstring &src, int base = 10);
    static float toFloat(const tstring &src);
    static double toDouble(const tstring &src);
    static tstring toBytes(const char c);
    static tstring toBytes(const tstring &src);
    template <typename typeChar>
    static bool strncmp(const typeChar *str1, const typeChar *str2) {
        tstring _s1 = toString(str1), _s2 = toString(str2);
        if (_s1.length() != _s2.length())
            return false;
        return TStringHelper::tolower(_s1) == TStringHelper::tolower(_s2) ? true : false;
    }
};
template <class typeStream>
class TFormatStream : public typeStream {
public:
    TFormatStream(void) : typeStream(), m_lpszFormat(&g_nEndFlags) {}
    ~TFormatStream(void) {}

    TFormatStream &format(const tstring &lpszFormat) {
        flushFormat();
        m_lpszFormat = lpszFormat.c_str();
        return outPrefix();
    }
    template <typename typeArg>
    TFormatStream &arg(const typeArg &val) {
        getSelf() << val;
        return outPrefix();
    }
    void flushFormat(void) {
        if (*m_lpszFormat) {
            getSelf() << m_lpszFormat;
            m_lpszFormat = &g_nEndFlags;
        }
    }

protected:
    TFormatStream &outLoop(void) {
        while (*m_lpszFormat == '%') {
            const char *lpPos = strchr(m_lpszFormat + 1, '%');
            if (lpPos != NULL) {
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
    TFormatStream &outPrefix(void) {
        const char *lpPos = strchr(m_lpszFormat, '%');
        if (lpPos != NULL) {
            typeStream::write(m_lpszFormat, static_cast<std::streamsize>(lpPos - m_lpszFormat));
            m_lpszFormat = lpPos + 1;
            if (*m_lpszFormat == '%' && *(m_lpszFormat + 1) != '%')
                return outLoop();
        } else
            flushFormat();
        return *this;
    }
    typeStream &getOutput(void) {
        return *this;
    }
    static char g_nEndFlags;
    const char *m_lpszFormat;
};
class TFmtString : public TFormatStream<tostrstream> {
public:
    typedef TFormatStream<tostrstream> typeParent;
    TFmtString(void) : typeParent() {}
    TFmtString(const char *lpszFormat) : typeParent() {
        typeParent::format(lpszFormat);
    }
    tstring toString(void) const {
        return typeParent::str();
    }
};
template <class typeStream>
char TFormatStream<typeStream>::g_nEndFlags = '\0';
NAMESPACE_END
#endif
