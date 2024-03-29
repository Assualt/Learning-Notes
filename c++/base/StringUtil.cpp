//
// Created by xhou on 2022/10/7.
//

#include "Ascii.h"
#include "StringUtil.h"

using namespace muduo::base;

int memCaseCmp(const char *s1, const char *s2, size_t len) {
    const unsigned char *us1 = reinterpret_cast<const unsigned char *>(s1);
    const unsigned char *us2 = reinterpret_cast<const unsigned char *>(s2);

    for (size_t i = 0; i < len; i++) {
        const int diff = int{static_cast<unsigned char>(ascii_tolower(us1[ i ]))} -
                         int{static_cast<unsigned char>(ascii_tolower(us2[ i ]))};
        if (diff != 0)
            return diff;
    }
    return 0;
}

bool util::EqualsIgnoreCase(std::string_view piece1, std::string_view piece2) noexcept {
    return (piece1.size() == piece2.size() && 0 == memCaseCmp(piece1.data(), piece2.data(), piece1.size()));
}

bool util::StartsWithIgnoreCase(std::string_view text, std::string_view prefix) noexcept {
    return (text.size() >= prefix.size()) && EqualsIgnoreCase(text.substr(0, prefix.size()), prefix);
}

bool util::EndsWithIgnoreCase(std::string_view text, std::string_view suffix) noexcept {
    return (text.size() >= suffix.size()) && EqualsIgnoreCase(text.substr(text.size() - suffix.size()), suffix);
}

std::string util::TrimLeft(const std::string &src, char ch) {
    std::string           temp = src;
    std::string::iterator p    = std::find_if(temp.begin(), temp.end(), [&ch](char c) { return 32 != c; });
    temp.erase(temp.begin(), p);
    return temp;
}

std::string util::TrimRight(const std::string &src, char ch) {
    std::string                   temp = src;
    std::string::reverse_iterator p    = find_if(temp.rbegin(), temp.rend(), [&ch](char c) { return ch != c; });
    temp.erase(p.base(), temp.end());
    return temp;
}

std::string util::Trim(const std::string &src, char ch) { return TrimRight(TrimLeft(src, ch), ch); }

std::vector<std::string> util::splitToVector(const std::string &strVal, char ch) {
    std::vector<std::string> result;
    std::string              temp;
    for (auto c : strVal) {
        if (c == ch) {
            temp = Trim(temp);
            if (temp.empty())
                continue;
            result.push_back(temp);
            temp.clear();
        } else {
            temp.push_back(c);
        }
    }
    temp = Trim(temp);
    if (!temp.empty())
        result.push_back(temp);
    else if (temp.empty())
        result.push_back(temp);
    return result;
}