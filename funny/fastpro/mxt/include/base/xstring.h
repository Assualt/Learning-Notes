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

inline void _ltrim(tstring &src, const char ch)  //
{
    size_t _pos = 0;
    while (src[_pos++] == ch)
        ;
    src.erase(0, _pos);
}
inline void _rtrim(tstring &src, const char ch)  //�Ƴ�����ո�
{
    size_t _pos = src.length() - 1;
    while (src[_pos--] == ch)
        ;
    src.erase(_pos);
}
inline void _trim(tstring &src, const char ch)  //�Ƴ�ǰ��ո�
{
    _ltrim(src, ch);
    _rtrim(src, ch);
}
template <class val_type>
extern tstring toString(val_type val) {
    std::stringstream buffer;
    buffer << val;
    return buffer.str();
}

class TFmtstring {
public:
    TFmtstring(void) {
        key_num = 0;
        m_strDel = '%';
    }
    TFmtstring(
            const char *lpszFormat,
            const char strDel = '%',
            size_t nmaxSize = 512) {
        key_num = 0;
        m_strFormat = tstring(lpszFormat);
        m_strDel = strDel;
        nMaxSize = nmaxSize;
        to_Array();
    }
    void setDel(const char strDel) {
        m_strDel = strDel;
    }
    void setFormat(const char *strFormat) {
        m_strFormat = strFormat;
        to_Array();
    }
    char *c_str();
    template <class val_type>
    TFmtstring &arg(val_type val) {
        if (key_num--) {
            lvallist.push_back(toString(val));
            return *this;
        }
        return *this;
    }
    ~TFmtstring() {
        m_strFormat.clear();
        m_strCache.clear();
        lkeylist.clear();
        lvallist.clear();
    }

    // src:格式化字符串 nSize大小 ch:填充字符 bFront:前面填充
    static tstring format(
            const char *src,
            size_t nSize,
            char ch,
            bool bFront = true) {
        if (strlen(src) == nSize)
            return src;
        if (strlen(src) > nSize)
            if (bFront)
                return tstring(src).substr(0, nSize);
            else
                return tstring(src).substr(strlen(src) - nSize);
        else if (bFront)
            return tstring(nSize - strlen(src), ch) + tstring(src);
        else
            return tstring(src) + tstring(nSize - strlen(src), ch);
    }

    template <typename typeChar>
    static tstring toFixed(
            typeChar src,
            size_t nSize,
            char ch,
            bool bFront = true) {
        return TFmtstring::format(toString(src).c_str(), nSize, ch, bFront);
    }

private:
    void to_Array();

private:
    int key_num, nMaxSize;
    char m_strDel;
    tstring m_strFormat;
    std::list<tstring> lkeylist, lvallist;
    tstring m_strCache;
};

class TStringHelper {
public:
    static tstring m_strEmptyString;
    static void split(
            std::list<tstring> &ret,
            const tstring &_src,
            const char _del = ',');
    static void splitBytes(
            std::list<tstring> &ret,
            const tstring &_src,
            size_t len = 8);
    static tstring ltrim(tstring src);    //去掉左方空格
    static tstring trim(tstring src);     //去掉左右方空格
    static tstring rtrim(tstring src);    //去掉右方空格
    static char toupper(char c);          //大写
    static char tolower(char c);          //小写
    static tstring tolower(tstring src);  //小写
    static tstring toupper(tstring src);  //大写
    static tstring replaceAll(
            tstring src,
            const char _src,
            const char _des);  //字符替换
    static tstring replaceAll(
            tstring src,
            const char *_src,
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
        return TStringHelper::tolower(_s1) == TStringHelper::tolower(_s2)
                ? true
                : false;
    }
};

NAMESPACE_END
#endif
