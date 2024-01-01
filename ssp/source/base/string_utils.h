//
// Created by 侯鑫 on 2023/12/27.
//

#ifndef SSP_TOOLS_STRING_UTILS_H
#define SSP_TOOLS_STRING_UTILS_H

#include <string>
#include <sstream>
#include <algorithm>

namespace ssp::base {
namespace util {
// StrContains()
//
// Returns whether a given string `haystack` contains the substring `needle`.
inline bool StrContains(std::string_view haystack, std::string_view needle) noexcept
{
    return haystack.find(needle, 0) != std::string_view::npos;
}

inline bool StrContains(std::string_view haystack, char needle) noexcept
{
    return haystack.find(needle) != std::string_view::npos;
}

// StartsWith()
//
// Returns whether a given string `text` begins with `prefix`.
inline bool StartsWith(std::string_view text, std::string_view prefix) noexcept
{
    return prefix.empty() ||
           (text.size() >= prefix.size() && memcmp(text.data(), prefix.data(), prefix.size()) == 0);
}

// EndsWith()
//
// Returns whether a given string `text` ends with `suffix`.
inline bool EndsWith(std::string_view text, std::string_view suffix) noexcept
{
    return suffix.empty() || (text.size() >= suffix.size() &&
                              memcmp(text.data() + (text.size() - suffix.size()), suffix.data(),
                                     suffix.size()) ==
                              0);
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

// TrimLeft()
//
// Returns the g give ASCII string text by using trim char `ch` on the left
std::string trimLeft(const std::string &src, char ch);

// TrimLeft()
//
// Returns the g give ASCII string text by using trim char `ch` on the right
std::string trimRight(const std::string &src, char ch);

// TrimLeft()
//
// Returns the g give ASCII string text by using trim char `ch` on the left|right
std::string trim(const std::string &src, char ch = ' ');

std::vector<std::string> SplitToVector(const std::string &strVal, char ch);

template<class typeString>
static typeString TrimRight(const typeString &strVal, const typeString &strSpace) {
    return strVal.substr(0, strVal.find_last_not_of(strSpace) + 1);
}

template<class typeString>
static typeString TrimLeft(const typeString &strVal, const typeString &strSpace) {
    typename typeString::size_type pos = strVal.find_first_not_of(strSpace);
    return (pos == typeString::npos) ? typeString() : typeString(strVal.substr(pos));
}

template<class typeString>
static typeString Trim(const typeString &strVal, const typeString &strSpace) {
    return TrimRight(TrimLeft(strVal, strSpace), strSpace);
}

inline long Count(const std::string &strVal, char ch)
{
    return std::count_if(strVal.begin(), strVal.end(), [ch](char c) { return ch == c; });
}
}

template<typename Target, typename Source, bool Same>
class lexical_cast_t {
public:
    static Target cast(const Source &arg)
    {
        Target ret;
        std::stringstream ss;
        if (!(ss << arg && ss >> ret && ss.eof()))
            throw std::bad_cast();

        return ret;
    }
};

template<typename Target, typename Source>
class lexical_cast_t<Target, Source, true> {
public:
    static Target cast(const Source &arg) { return arg; }
};

template<typename Source>
class lexical_cast_t<std::string, Source, false> {
public:
    static std::string cast(const Source &arg)
    {
        std::ostringstream ss;
        ss << arg;
        return ss.str();
    }
};

template<typename Target>
class lexical_cast_t<Target, std::string, false> {
public:
    [[maybe_unused]] static Target cast(const std::string &arg)
    {
        Target ret;
        std::istringstream ss(arg);
        if (!(ss >> ret && ss.eof()))
            throw std::bad_cast();
        return ret;
    }
};

template<typename T1, typename T2>
struct is_same {
    static const bool value = false;
};

template<typename T>
struct is_same<T, T> {
    static const bool value = true;
};

template<typename Target, typename Source>
Target lexical_cast(const Source &arg)
{
    return lexical_cast_t<Target, Source, is_same<Target, Source>::value>::cast(arg);
}

}
#endif //SSP_TOOLS_STRING_UTILS_H
