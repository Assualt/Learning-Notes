#pragma clang diagnostic push
#pragma ide diagnostic   ignored "OCUnusedGlobalDeclarationInspection"
#pragma once
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <vector>

namespace muduo::base {
using cstring = const std::string &;
using vstring = std::vector<std::string>;
using Section = std::map<std::string, std::string>;

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
    return std::count_if(strVal.begin(), strVal.end(), [ ch ](char c) { return ch == c; });
}

enum FILE_TYPE { Type_Unknown, Type_File, Type_Dir, Type_Other };
static FILE_TYPE getFileType(cstring file) {
    struct stat st {};
    if (stat(file.c_str(), &st) == -1)
        return Type_Unknown;
    switch (st.st_mode & S_IFMT) {
        case S_IFREG:
            return Type_File;
        case S_IFDIR:
            return Type_Dir;
    }
    return Type_Other;
}
static bool startWith(cstring src, cstring prefix) { return src.substr(0, prefix.size()) == prefix; }
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
    static Target cast(const Source &arg) { return arg; }
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
    [[maybe_unused]] static Target cast(const std::string &arg) {
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
    explicit ConfigureManager(cstring confPath, cstring suffix = ".cf")
        : m_Suffix(suffix) {
        GetAllFiles(confPath, suffix);
    }
    void Init();

public:
    void changeAccessPath(cstring confPath);

    [[nodiscard]] std::string getString(cstring strDefault, cstring prefix) const;
    [[nodiscard]] bool        getBool(bool nDefault, cstring prefix) const;
    [[nodiscard]] int         getInt(int nDefault, cstring prefix) const;
    [[nodiscard]] float       getFloat(float fDefault, cstring prefix) const;
    [[nodiscard]] double      getDouble(double dDefault, cstring prefix) const;
    [[nodiscard]] long        getLong(long lDefault, cstring prefix) const;
    [[nodiscard]] Section     getSection(cstring prefix) const;

protected:
    void GetAllFiles(cstring path, cstring suffix);
    void InitFile(cstring file);

protected:
    vstring                            m_vAllConfFiles;
    std::map<std::string, std::string> m_mConfigKeyValMapper;
    std::string                        m_strPrefix;
    std::string                        m_Suffix;
};

} // namespace muduo::base
#pragma clang diagnostic pop