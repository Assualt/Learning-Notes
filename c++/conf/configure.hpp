#pragma once
#include "logging.h"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sys/stat.h>
#include <vector>
using namespace std;

namespace conf {
using cstring = const std::string &;
using vstring = std::vector<std::string>;

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
    ConfigureManager(cstring confPath, cstring suffix = ".cf");
    void init();

public:
    void changeAccessPath(cstring confpath);

    std::string getString(cstring strdefault, cstring prefix) const;
    bool        getBool(bool ndefault, cstring prefix) const;
    int         getInt(int ndefault, cstring prefix) const;
    float       getFloat(float fdefault, cstring prefix) const;
    double      getDouble(double ddefault, cstring prefix) const;
    long        getLong(long ldefault, cstring prefix) const;

protected:
    void GetAllFiles(cstring path, cstring suffix);
    void InitFile(cstring file);

protected:
    vstring                            m_vAllConfFiles;
    std::map<std::string, std::string> m_ConfigKeyValMapper;
    std::string                        m_strPrefix;
};

ConfigureManager::ConfigureManager(cstring conf, cstring suffix) {
    GetAllFiles(conf, suffix);
}

void ConfigureManager::GetAllFiles(cstring path, cstring suffix) {
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr)
        return;
    struct dirent *dr;
    while ((dr = readdir(dir)) != nullptr) {
        if (strcasecmp(dr->d_name, ".") == 0 || strcasecmp(dr->d_name, "..") == 0)
            continue;
        auto fileName = path + "/" + dr->d_name;
        auto filetype = detail::getFileType(fileName);
        if (filetype == detail::Type_Dir)
            GetAllFiles(fileName, suffix);
        else if (filetype == detail::Type_File && detail::endWith(fileName, suffix))
            m_vAllConfFiles.emplace_back(fileName);
    }
    closedir(dir);
}

void ConfigureManager::init() {
    for (auto file : m_vAllConfFiles)
        InitFile(file);
}

void ConfigureManager::InitFile(cstring file) {
    std::string Prefix = "/";
    size_t      t      = file.rfind("/");
    if (t != std::string::npos)
        Prefix.append(detail::trimRight(file.substr(t + 1), std::string(".cf")));
    else
        Prefix.append(detail::trimRight(file, std::string(".cf")));
    std::string   strLine;
    std::ifstream fin(file);
    if (!fin.is_open())
        return;

    std::string strSectionKey, strKey, strVal;
    bool        bFindSection = false;
    while (getline(fin, strLine)) {
        strLine = detail::trim(strLine, std::string(" "));
        if (strLine.empty())
            continue;
        else if (detail::startWith(strLine, "#"))
            continue;
        size_t nPos;
        if ((nPos = strLine.find("#")) != std::string::npos)
            strLine = strLine.substr(0, nPos);
        if (strLine.back() == ']' && strLine.front() == '[' && detail::count(strLine, '[') == 1 && detail::count(strLine, ']') == 1) {
            auto temp = strLine.substr(1, strLine.size() - 2);
            if (temp.empty())
                continue;
            strSectionKey = Prefix + "/" + temp;
            bFindSection  = true;
        } else if (bFindSection && detail::count(strLine, '=')) {
            auto pos = strLine.find('=');
            strKey   = detail::trim(strLine.substr(0, pos), std::string(" "));
            strVal   = detail::trim(strLine.substr(pos + 1), std::string(" "));
            strKey   = detail::trim(strKey, std::string("'"));
            strVal   = detail::trim(strVal, std::string("'"));
            strKey   = detail::trim(strKey, std::string("\""));
            strVal   = detail::trim(strVal, std::string("\""));
            if (!strKey.empty() && !strVal.empty()) {
                m_ConfigKeyValMapper[ strSectionKey + "/" + strKey ] = strVal;
                logger.info("success input [%s]->[%s]", strSectionKey + "/" + strKey, strVal);
            }
        }
    }
    fin.close();
}

bool ConfigureManager::getBool(bool bdefault, cstring prefix) const {
    std::string Prefix = m_strPrefix + prefix;
    if (m_ConfigKeyValMapper.find(Prefix) == m_ConfigKeyValMapper.end())
        return bdefault;
    if(strcasecmp(m_ConfigKeyValMapper.at(Prefix).c_str(), "true") == 0)
        return true;
    else if(strcasecmp(m_ConfigKeyValMapper.at(Prefix).c_str(), "false") == 0)
        return false;
    return detail::lexical_cast<bool, std::string>(m_ConfigKeyValMapper.at(Prefix));
}

std::string ConfigureManager::getString(cstring strdefault, cstring prefix) const {
    std::string Prefix = m_strPrefix + prefix;
    if (m_ConfigKeyValMapper.find(Prefix) == m_ConfigKeyValMapper.end())
        return strdefault;
    return m_ConfigKeyValMapper.at(Prefix);
}
int ConfigureManager::getInt(int ndefault, cstring prefix) const {
    std::string Prefix = m_strPrefix + prefix;
    if (m_ConfigKeyValMapper.find(Prefix) == m_ConfigKeyValMapper.end())
        return ndefault;
    return detail::lexical_cast<int, std::string>(m_ConfigKeyValMapper.at(Prefix));
}
float ConfigureManager::getFloat(float fdefault, cstring prefix) const {
    std::string Prefix = m_strPrefix + prefix;
    if (m_ConfigKeyValMapper.find(Prefix) == m_ConfigKeyValMapper.end())
        return fdefault;
    return detail::lexical_cast<float, std::string>(m_ConfigKeyValMapper.at(Prefix));
}
double ConfigureManager::getDouble(double ddefault, cstring prefix) const {
    std::string Prefix = m_strPrefix + prefix;
    if (m_ConfigKeyValMapper.find(Prefix) == m_ConfigKeyValMapper.end())
        return ddefault;
    return detail::lexical_cast<double, std::string>(m_ConfigKeyValMapper.at(Prefix));
}
long ConfigureManager::getLong(long ldefault, cstring prefix) const {
    std::string Prefix = m_strPrefix + prefix;
    if (m_ConfigKeyValMapper.find(Prefix) == m_ConfigKeyValMapper.end())
        return ldefault;
    return detail::lexical_cast<long, std::string>(m_ConfigKeyValMapper.at(Prefix));
}

void ConfigureManager::changeAccessPath(cstring confpath) {
    m_strPrefix = confpath;
}

} // namespace conf
