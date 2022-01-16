#ifndef _X_STRING_H_2018_12_14_
#define _X_STRING_H_2018_12_14_

#include <list>
#include "xmtdef.h"
#include <string.h>
#include "texception.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

using namespace std;
NAMESPACE_BEGIN

typedef std::string tstring;
typedef std::wstring twstring;
typedef std::istream tistream;
typedef std::ostream tostream;
typedef std::ifstream tifstream;
typedef std::ofstream tofstream;
typedef std::ostringstream tostrstream;

template <class val_type>
extern tstring toString(const val_type &val) {
    std::stringstream buffer;
    buffer << val;
    return buffer.str();
}
template <class val_type>
extern twstring toWString(const val_type &val) {
    std::wstringstream buffer;
    buffer << val;
    return buffer.str();
}
class TStringHelper {
public:
    static tstring m_strEmptyString;

    /**
     * @function: split
     * @param src: input string
     * @param delimeter: the delimeter char
     * @param times: the spilt times (default -1)
     * @return: return string vector
     */
    static std::vector<tstring> split(const tstring &src, char delimeter = ',', int times = -1);

    /**
     * @function: ltrim
     * @param src: the input string
     * @return: trim left char
     */
    static tstring ltrim(const tstring &src, char ch = ' ');

    /**
     * @function: rtrim
     * @param src: the input string
     * @return: trim right and left char
     */
    static tstring trim(const tstring &src, char ch = ' ');

    /**
     * @function: rtrim
     * @param src: the input string const
     * @return: trim right and left ch
     */
    static tstring rtrim(const tstring &src, char ch = ' ');  //去掉右方空格

    /**
     * @function: toupper
     * @param c: the char
     * @return: upper char
     */
    static char toupper(char c);

    /**
     * @function: tolower
     * @param c: the char
     * @return: lower char
     */
    static char tolower(char c);

    /**
     * @function: tolower
     * @param c: the input tstring reference
     * @return: lower tstring
     */
    static tstring tolower(tstring &src);

    /**
     * @function: toupper
     * @param src: the input tstring reference
     * @return: upper tstring
     */
    static tstring toupper(tstring &src);

    /**
     * @function: tolower
     * @param c: the input tstring const
     * @return: lower tstring
     */
    static tstring tolower(const tstring &src);

    /**
     * @function: toupper
     * @param src: the input tstring const
     * @return: upper tstring
     */
    static tstring touppder(const tstring &src);

    /**
     * @brief
     * @function: replaceAll
     * @param src: the input string reference
     * @param _src: the replace char
     * @param _des: the dest char
     * @return: After replace all char tstring
     */
    static tstring replaceAll(tstring &src, char _src, char _des);

    /**
     * @brief: Not Recommend Methods
     * @function: replaceAll
     * @param src: the input string reference
     * @param _src: the replace char
     * @param _des: the dest char
     * @return: After replace all char tstring
     */
    static tstring replaceAll(tstring &src, const char *_src, const char *_des);

    /**
     * @brief: check string startWith some prefix
     * @function: startWith
     * @param src: the input string const
     * @param prefix: the prefix string
     * @return: if src start with prefix string  return true , else false
     */
    static bool startWith(const tstring &src, const char *prefix);

    /**
     * @brief: check string endWith some prefix
     * @function: endWith
     * @param src: the input string const
     * @param backfix: the backfix string
     * @return: if src ends with backfix string return true , else false
     */
    static bool endWith(const tstring &src, const char *backfix);

    /**
     * @brief: convert tstring to int with base
     * @function: toInt
     * @param src: the input tstring
     * @param base: the number base
     * @return: the Int Val
     * @throws: may cause exception with std::__throw_out_of_range
     */
    static int toInt(const tstring &src, int base = 10);

    /**
     * @brief: convert tstring to float with base
     * @function: toFloat
     * @param src: the input tstring
     * @param base: the number base
     * @return: the Float Val
     * @throws: may cause exception with std::__throw_out_of_range
     */
    static float toFloat(const tstring &src);

    /**
     * @brief: convert tstring to double with base
     * @function: toFloat
     * @param src: the input tstring
     * @param base: the number base
     * @return: the Double Val
     * @throws: may cause exception with std::__throw_out_of_range
     */
    static double toDouble(const tstring &src);

    /**
     * @brief: diff the string a1 and b1 in ignore case
     * @function: strncmp
     * @param str1, str2: the input string, must be the sample type
     * @return: if equals is true, else false
     */
    template <typename typeChar>
    static bool strncmp(const typeChar *str1, const typeChar *str2) {
        tstring _s1 = toString(str1), _s2 = toString(str2);
        if (_s1.length() != _s2.length())
            return false;
        return TStringHelper::tolower(_s1) == TStringHelper::tolower(_s2);
    }

    /**
     * @brief: Capitalizes a String changing the first character to title case as per Character.toTitleCase(int). No
     * other characters are changed.
     * @function: capitalize
     * @param: the input string const
     * @return: the Capitailized String
     */
    static tstring capitalize(const tstring &str);

    /**
     * @brief: Capitalizes a String changing the first character to title case as per Character.toTitleCase(int). other
     * characters change to lower case
     * @function: title
     * @param: the input string const
     * @return: the Capitailized String
     */
    static tstring title(const tstring &str);

    /**
     * @brief: how many char do the string contain
     * @function: count
     * @param: the input string const
     * @return: the count of char
     */
    static size_t count(const tstring &src, char ch);
};
template <class typeStream>
class TFormatStream : public typeStream {
public:
    TFormatStream() : typeStream(), m_lpszFormat(&g_nEndFlags) {}

    TFormatStream &format(const char *lpszFormat) {  // 此处不能使用lpszFormat为string类型,否则会出现错误
        flushFormat();
        m_lpszFormat = lpszFormat;
        return outPrefix();
    }
    template <typename typeArg>
    TFormatStream &arg(const typeArg &val) {
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
    TFormatStream &outLoop() {
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
    TFormatStream &outPrefix() {
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
    typeStream &getSelf() {
        return *this;
    }

protected:
    static char g_nEndFlags;
    const char *m_lpszFormat;
};
class TFmtString : public TFormatStream<tostrstream> {
public:
    typedef TFormatStream<tostrstream> typeParent;
    TFmtString() : typeParent() {}
    explicit TFmtString(const char *lpszFormat) : typeParent() {
        typeParent::format(lpszFormat);
    }
    tstring toString() const {
        return typeParent::str();
    }
};
template <class typeStream>
char TFormatStream<typeStream>::g_nEndFlags = '\0';
NAMESPACE_END
#endif
