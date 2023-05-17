#pragma once
#include <algorithm>
#include <iostream>
#include <vector>
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
        return std::count_if(strVal.begin(), strVal.end(), [ch](char c) { return ch == c; });
    }

    static std::vector<std::string> split(const std::string &src, char divider) {
        std::vector<std::string> result;
        std::string              temp;
        for (auto &ch : src) {
            if (ch == divider) {
                if (!trim(temp, std::string(" ")).empty())
                    result.push_back(trim(temp, std::string(" ")));
                temp.clear();
            } else {
                temp.push_back(ch);
            }
        }
        if (!temp.empty())
            result.push_back(trim(temp, std::string(" ")));
        return result;
    }
};

} // namespace mail