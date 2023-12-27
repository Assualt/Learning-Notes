//
// Created by 侯鑫 on 2023/12/27.
//

#include "string_utils.h"
#include "ascii.h"

namespace ssp::base::util {

int memCaseCmp(const char *s1, const char *s2, size_t len)
{
    const unsigned char *us1 = reinterpret_cast<const unsigned char *>(s1);
    const unsigned char *us2 = reinterpret_cast<const unsigned char *>(s2);

    for (size_t i = 0; i < len; i++) {
        const int diff = int{static_cast<unsigned char>(ascii_tolower(us1[i]))} -
                         int{static_cast<unsigned char>(ascii_tolower(us2[i]))};
        if (diff != 0)
            return diff;
    }
    return 0;
}

bool EqualsIgnoreCase(std::string_view piece1, std::string_view piece2) noexcept
{
    return (piece1.size() == piece2.size() && 0 == memCaseCmp(piece1.data(), piece2.data(), piece1.size()));
}

bool StartsWithIgnoreCase(std::string_view text, std::string_view prefix) noexcept
{
    return (text.size() >= prefix.size()) && EqualsIgnoreCase(text.substr(0, prefix.size()), prefix);
}

bool EndsWithIgnoreCase(std::string_view text, std::string_view suffix) noexcept
{
    return (text.size() >= suffix.size()) && EqualsIgnoreCase(text.substr(text.size() - suffix.size()), suffix);
}

std::string trimLeft(const std::string &src, char ch)
{
    std::string temp = src;
    std::string::iterator p = std::find_if(temp.begin(), temp.end(), [&ch](char c) { return ch != c; });
    temp.erase(temp.begin(), p);
    return temp;
}

std::string trimRight(const std::string &src, char ch)
{
    std::string temp = src;
    std::string::reverse_iterator p = find_if(temp.rbegin(), temp.rend(), [&ch](char c) { return ch != c; });
    temp.erase(p.base(), temp.end());
    return temp;
}

std::string trim(const std::string &src, char ch)
{
    auto myStr = trimLeft(src, ch);
    return trimRight(myStr, ch);
}

std::vector<std::string> SplitToVector(const std::string &strVal, char ch)
{
    std::vector<std::string> result;
    std::string temp;
    for (auto c: strVal) {
        if (c == ch) {
            temp = trim(temp);
            if (temp.empty()) continue;
            result.push_back(temp);
            temp.clear();
        } else {
            temp.push_back(c);
        }
    }
    temp = trim(temp);
    if (!temp.empty())
        result.push_back(temp);
    else if (temp.empty())
        result.push_back(temp);
    return result;
}
}