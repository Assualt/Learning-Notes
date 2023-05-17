#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
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