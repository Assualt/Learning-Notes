//
// Created by xhou on 2022/10/7.
//

#ifndef MUDUO_BASE_TOOLS_STRINGUTILS_H
#define MUDUO_BASE_TOOLS_STRINGUTILS_H

#include <cstring>
#include <string_view>

namespace muduo::base::util {

// StrContains()
//
// Returns whether a given string `haystack` contains the substring `needle`.
inline bool StrContains(std::string_view haystack, std::string_view needle) noexcept {
    return haystack.find(needle, 0) != haystack.npos;
}

inline bool StrContains(std::string_view haystack, char needle) noexcept {
    return haystack.find(needle) != haystack.npos;
}

// StartsWith()
//
// Returns whether a given string `text` begins with `prefix`.
inline bool StartsWith(std::string_view text, std::string_view prefix) noexcept {
    return prefix.empty() || (text.size() >= prefix.size() && memcmp(text.data(), prefix.data(), prefix.size()) == 0);
}

// EndsWith()
//
// Returns whether a given string `text` ends with `suffix`.
inline bool EndsWith(std::string_view text, std::string_view suffix) noexcept {
    return suffix.empty() || (text.size() >= suffix.size() &&
                              memcmp(text.data() + (text.size() - suffix.size()), suffix.data(), suffix.size()) == 0);
}

// EqualsIgnoreCase()
//
// Returns whether given ASCII strings `piece1` and `piece2` are equal, ignoring
// case in the comparison.
bool EqualsIgnoreCase(std::string_view piece1, std::string_view piece2) noexcept;

// StartsWithIgnoreCase()
//
// Returns whether a given ASCII string `text` starts with `prefix`,
// ignoring case in the comparison.
bool StartsWithIgnoreCase(std::string_view text, std::string_view prefix) noexcept;

// EndsWithIgnoreCase()
//
// Returns whether a given ASCII string `text` ends with `suffix`, ignoring
// case in the comparison.
bool EndsWithIgnoreCase(std::string_view text, std::string_view suffix) noexcept;

} // namespace muduo::base::util

#endif // MUDUO_BASE_TOOLS_STRINGUTILS_H
