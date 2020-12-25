#pragma once
#include <algorithm>
#include <iostream>
namespace mail {

class Utils {
public:
    template <class typeString> static typeString trimRight(const typeString &strVal, const typeString &strSpace) {
        return strVal.substr(0, strVal.find_last_not_of(strSpace) + 1);
    }
    template <class typeString> static typeString trimLeft(const typeString &strVal, const typeString &strSpace) {
        typename typeString::size_type pos = strVal.find_first_not_of(strSpace);
        return (pos == typeString::npos) ? typeString() : typeString(strVal.substr(pos));
    }
    template <class typeString> static typeString trim(const typeString &strVal, const typeString &strSpace) {
        return trimLeft(trimRight(strVal, strSpace), strSpace);
    }
    static int count(const std::string &strVal, char ch) {
        return std::count_if(strVal.begin(), strVal.end(), [ ch ](char c) { return ch == c; });
    }
};

} // namespace mail