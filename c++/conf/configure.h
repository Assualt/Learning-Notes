#pragma once
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <vector>

namespace conf {
using cstring = const std::string &;
using vstring = std::vector<std::string>;
using section = std::map<std::string, std::string>;

namespace detail {
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
enum FILE_TYPE { Type_Unknown, Type_File, Type_Dir, Type_Other };
static FILE_TYPE getFileType(cstring file) {
    struct stat st;
    if (stat(file.c_str(), &st) == -1)
        return Type_Unknown;
    switch (st.st_mode & S_IFMT) {
        case S_IFREG:
            return Type_File;
        case S_IFDIR:
            return Type_Dir;
        default:
            return Type_Other;
    }
    return Type_Other;
}
static bool startWith(cstring src, cstring prefix) {
    return src.substr(0, prefix.size()) == prefix;
}
static bool endWith(cstring src, cstring backfix) {
    return src.substr(src.size() - backfix.size(), backfix.size()) == backfix;
}
template <typename Target, typename Source, bool Same> class lexical_cast_t {
public:
    static Target cast(const Source &arg) {
        Target            ret;
        std::stringstream ss;
        if (!(ss << arg && ss >> ret && ss.eof()))
            throw std::bad_cast();

        return ret;
    }
};

template <typename Target, typename Source> class lexical_cast_t<Target, Source, true> {
public:
    static Target cast(const Source &arg) {
        return arg;
    }
};

template <typename Source> class lexical_cast_t<std::string, Source, false> {
public:
    static std::string cast(const Source &arg) {
        std::ostringstream ss;
        ss << arg;
        return ss.str();
    }
};

template <typename Target> class lexical_cast_t<Target, std::string, false> {
public:
    static Target cast(const std::string &arg) {
        Target             ret;
        std::istringstream ss(arg);
        if (!(ss >> ret && ss.eof()))
            throw std::bad_cast();
        return ret;
    }
};

template <typename T1, typename T2> struct is_same { static const bool value = false; };

template <typename T> struct is_same<T, T> { static const bool value = true; };

template <typename Target, typename Source> Target lexical_cast(const Source &arg) {
    return lexical_cast_t<Target, Source, detail::is_same<Target, Source>::value>::cast(arg);
}
} // namespace detail

class ConfigureManager {
public:
    ConfigureManager(cstring confPath, cstring suffix = ".cf")
        : m_Suffix(suffix) {
        GetAllFiles(confPath, suffix);
    }
    void init();

public:
    void changeAccessPath(cstring confpath);

    std::string getString(cstring strdefault, cstring prefix) const;
    bool        getBool(bool ndefault, cstring prefix) const;
    int         getInt(int ndefault, cstring prefix) const;
    float       getFloat(float fdefault, cstring prefix) const;
    double      getDouble(double ddefault, cstring prefix) const;
    long        getLong(long ldefault, cstring prefix) const;
    section     getSection(cstring prefix) const;

protected:
    void GetAllFiles(cstring path, cstring suffix);
    void InitFile(cstring file);

protected:
    vstring                            m_vAllConfFiles;
    std::map<std::string, std::string> m_ConfigKeyValMapper;
    std::string                        m_strPrefix;
    std::string                        m_Suffix;
};
} // namespace conf
