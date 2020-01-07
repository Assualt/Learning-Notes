#include "base/xstring.h"
#include "base/xstringbuilder.h"
#include <algorithm>
#include <iostream>
NAMESPACE_BEGIN
// static args Init
tstring TStringHelper::m_strEmptyString = "";
void TStringHelper::split(std::list<tstring> &ret, const tstring &_src, const char _del) {
    if (_src.empty())
        return;
    ret.clear();
    size_t _pos = 0, _len = _src.length(), _pBegin = 0;
    for (; _pos < _len; _pos++) {
        if (_src[_pos] == _del) {
            ret.push_back(_src.substr(_pBegin, _pos - _pBegin));
            _pBegin = _pos + 1;
        }
    }
    if (_pBegin != _src.size())
        ret.push_back(_src.substr(_pBegin));
}
void TStringHelper::splitBytes(std::list<tstring> &ret, const tstring &_src, size_t len) {
    ret.clear();
    size_t _pos = 0, _len = _src.size(), _left = _src.size();
    if (_len < len) {
        ret.push_back(_src);
        return;
    }
    for (; _pos < _len;) {
        if (_left <= len)
            ret.push_back(_src.substr(_pos));
        else
            ret.push_back(_src.substr(_pos, len));
        _pos += 8;
        _left -= len;
    }
}
tstring TStringHelper::ltrim(tstring src)  //移除前方空格
{
    string::iterator p = std::find_if(src.begin(), src.end(), not1(ptr_fun(::isspace)));
    src.erase(src.begin(), p);
    return src;
}
tstring TStringHelper::trim(tstring src)  //移除前后空格
{
    return TStringHelper::rtrim(TStringHelper::ltrim(src));
}
tstring TStringHelper::rtrim(tstring src)  //移除后面空格
{
    string::reverse_iterator p = find_if(src.rbegin(), src.rend(), not1(ptr_fun(::isspace)));
    src.erase(p.base(), src.end());
    return src;
}
char TStringHelper::toupper(char c)  //大写
{
    if (c >= 'a' && c <= 'z')
        c -= ('a' - 'A');
    return c;
}
char TStringHelper::tolower(char c)  //小写
{
    if (c >= 'A' && c <= 'Z')
        c += ('a' - 'A');
    return c;
}
tstring TStringHelper::tolower(tstring src)  //小写
{
    std::transform(src.begin(), src.end(), src.begin(), ::tolower);
    return src;
}
tstring TStringHelper::toupper(tstring src)  //大写
{
    std::transform(src.begin(), src.end(), src.begin(), ::toupper);
    return src;
}

tstring TStringHelper::replaceAll(tstring src, const char _src, const char _des) {
    //[=] 以值的方式捕获所有的外部自动变量。
    std::transform(src.begin(), src.end(), src.begin(), [=](char ch) -> char { return ch == _src ? _des : ch; });
    return src;
}
tstring TStringHelper::replaceAll(tstring src, const char *_src, const char *_des) {
    if (NULL == _src || NULL == _des)
        return src;
    size_t pBegin;
    while ((pBegin = src.find(_src)) != tstring::npos)
        src = src.substr(0, pBegin) + tstring(_des) + src.substr(pBegin + strlen(_src));
    return src;
}
bool TStringHelper::startWith(const tstring &src, const char *prefix) {
    if (NULL == prefix)
        return false;
    return src.substr(0, strlen(prefix)) == prefix;
}
bool TStringHelper::endWith(const tstring &src, const char *backfix) {
    if (NULL == backfix)
        return false;
    return src.substr(src.size() - strlen(backfix), strlen(backfix)) == backfix;
}
int TStringHelper::toInt(const tstring &src, int base) {
    return std::stoi(src, nullptr, base);
}
float TStringHelper::toFloat(const tstring &src) {
    return std::stof(src, nullptr);
}
double TStringHelper::toDouble(const tstring &src) {
    return std::stod(src, nullptr);
}
tstring TStringHelper::toBytes(const char c) {
    tstring byteArry;
    byteArry.reserve(8);
    int i = 0x1;
    size_t _pos = 8;
    while (_pos--)
        byteArry.push_back((c & (i << _pos)) == 0 ? '0' : '1');
    return byteArry;
}
tstring TStringHelper::toBytes(const tstring &src) {
    tstring byteArry;
    byteArry.clear();
    size_t _pos = 0, _len = src.size();
    for (; _pos < _len; _pos++) {
        tstring temp;
        temp = TStringHelper::toBytes(src[_pos]);
        byteArry += temp;
    }
    return byteArry;
}

NAMESPACE_END
