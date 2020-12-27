#include "base/tstring.h"
#include "base/tstringbuilder.h"
#include <algorithm>
#include <iostream>
NAMESPACE_BEGIN
// static args Init
tstring TStringHelper::m_strEmptyString = "";

std::vector<tstring> TStringHelper::split(const tstring &src, char delimeter, int times) {
    if (src.empty())
        return {};
    std::vector<tstring> result{};
    tstring temp;
    for (auto &ch : src) {
        if (ch == delimeter) {
            if (times == -1) {
                result.push_back(temp);
                temp.clear();
            } else if (times) {
                result.push_back(temp);
                times--;
                temp.clear();
            } else {
                temp.push_back(ch);
            }
        } else {
            temp.push_back(ch);
        }
    }
    if (!temp.empty())
        result.push_back(temp);
    return result;
}
tstring TStringHelper::ltrim(const tstring &src, char ch) {
    tstring temp = src;
    tstring::iterator p = std::find_if(temp.begin(), temp.end(), [&ch](char c) { return ch != c; });
    temp.erase(temp.begin(), p);
    return temp;
}
tstring TStringHelper::rtrim(const tstring &src, char ch) {
    string temp = src;
    string::reverse_iterator p = find_if(temp.rbegin(), temp.rend(), [&ch](char c) { return ch != c; });
    temp.erase(p.base(), temp.end());
    return temp;
}
tstring TStringHelper::trim(const tstring &src, char ch) {
    return TStringHelper::rtrim(TStringHelper::ltrim(src, ch), ch);
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
tstring TStringHelper::tolower(tstring &src)  //小写
{
    std::transform(src.begin(), src.end(), src.begin(), ::tolower);
    return src;
}
tstring TStringHelper::toupper(tstring &src)  //大写
{
    std::transform(src.begin(), src.end(), src.begin(), ::toupper);
    return src;
}
tstring TStringHelper::tolower(const tstring &src) {
    tstring temp = src;
    std::transform(src.begin(), src.end(), temp.begin(), ::tolower);
    return temp;
}
tstring TStringHelper::touppder(const tstring &src) {
    tstring temp = src;
    std::transform(src.begin(), src.end(), temp.begin(), ::toupper);
    return temp;
}

tstring TStringHelper::replaceAll(tstring &src, const char _src, const char _des) {
    //[=] 以值的方式捕获所有的外部自动变量。
    std::transform(src.begin(), src.end(), src.begin(), [=](char ch) -> char { return ch == _src ? _des : ch; });
    return src;
}
tstring TStringHelper::replaceAll(tstring &src, const char *_src, const char *_des) {
    if (nullptr == _src || nullptr == _des)
        return src;
    size_t pBegin;
    while ((pBegin = src.find(_src)) != tstring::npos)
        src = src.substr(0, pBegin) + tstring(_des) + src.substr(pBegin + strlen(_src));
    return src;
}
bool TStringHelper::startWith(const tstring &src, const char *prefix) {
    if (nullptr == prefix)
        return false;
    return src.substr(0, strlen(prefix)) == prefix;
}
bool TStringHelper::endWith(const tstring &src, const char *backfix) {
    if (nullptr == backfix)
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
tstring TStringHelper::capitalize(const tstring &str) {
    if (str.empty())
        return str;
    tstring temp = str;
    temp[0] = toupper(temp[0]);
    return temp;
}
size_t TStringHelper::count(const tstring &src, char ch){
    return std::count(src.begin(), src.end(), ch);
}
tstring TStringHelper::title(const tstring &str) {
    tstring temp = str;
    bool FindSpace = false;
    for (size_t i = 0; i < temp.size(); ++i) {
        if (FindSpace || i == 0) {
            temp[i] = TStringHelper::toupper(temp[i]);
            FindSpace = false;
        } else if (temp[i] == ' ') {
            FindSpace = true;
        } else {
            temp[i] = TStringHelper::tolower(temp[i]);
            FindSpace = false;
        }
    }
    return temp;
}

NAMESPACE_END
