#include "Configure.h"
#include "StringUtil.h"
using namespace muduo;
using namespace muduo::base;

namespace {
enum FILE_TYPE { Type_Unknown, Type_File, Type_Dir, Type_Other };
FILE_TYPE getFileType(cstring file) {
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
} // namespace

void ConfigureManager::GetAllFiles(cstring path, cstring suffix) {
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr)
        return;
    struct dirent *dr;
    while ((dr = readdir(dir)) != nullptr) {
        if (strcasecmp(dr->d_name, ".") == 0 || strcasecmp(dr->d_name, "..") == 0) {
            continue;
        }
        auto fileName = path + "/" + dr->d_name;
        auto filetype = getFileType(fileName);
        if (filetype == Type_Dir) {
            GetAllFiles(fileName, suffix);
        } else if (filetype == Type_File && util::EndsWith(fileName, suffix)) {
            m_vAllConfFiles.emplace_back(fileName);
        }
    }
    closedir(dir);
}

void ConfigureManager::Init() {
    for (auto file : m_vAllConfFiles) {
        InitFile(file);
    }
}

void ConfigureManager::InitFile(cstring file) {
    std::string strPrefix = "/";
    size_t      pos       = file.rfind("/");
    if (pos != std::string::npos) {
        strPrefix.append(util::trimRight(file.substr(pos + 1), m_Suffix));
    } else {
        strPrefix.append(util::trimRight(file, m_Suffix));
    }
    std::string   strLine;
    std::ifstream fin(file);
    if (!fin.is_open()) {
        return;
    }

    std::string strSectionKey, strKey, strVal;
    bool        bFindSection = false;
    while (getline(fin, strLine)) {
        strLine = util::trim(strLine, std::string(" "));
        if (strLine.empty() || util::StartsWith(strLine, "#")) {
            continue;
        }

        size_t nPos;
        if ((nPos = strLine.find("#")) != std::string::npos)
            strLine = strLine.substr(0, nPos);
        if (strLine.back() == ']' && strLine.front() == '[' && util::count(strLine, '[') == 1 &&
            util::count(strLine, ']') == 1) {
            auto temp = strLine.substr(1, strLine.size() - 2);
            if (temp.empty()) {
                continue;
            }
            strSectionKey = strPrefix + "/" + temp;
            bFindSection  = true;
        } else if (bFindSection && util::count(strLine, '=')) {
            auto pos1 = strLine.find('=');
            strKey    = util::trim(strLine.substr(0, pos1), std::string(" "));
            strVal    = util::trim(strLine.substr(pos1 + 1), std::string(" "));
            strKey    = util::trim(strKey, std::string("'"));
            strVal    = util::trim(strVal, std::string("'"));
            strKey    = util::trim(strKey, std::string("\""));
            strVal    = util::trim(strVal, std::string("\""));
            if (!strKey.empty() && !strVal.empty()) {
                m_mConfigKeyValMapper[ strSectionKey + "/" + strKey ] = strVal;
            }
        }
    }
    fin.close();
}

bool ConfigureManager::getBool(bool bDefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return bDefault;
    if (strcasecmp(m_mConfigKeyValMapper.at(strPrefix).c_str(), "true") == 0)
        return true;
    else if (strcasecmp(m_mConfigKeyValMapper.at(strPrefix).c_str(), "false") == 0)
        return false;
    return util::lexical_cast<bool, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

[[maybe_unused]] std::string ConfigureManager::getString(cstring strDefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return strDefault;
    return m_mConfigKeyValMapper.at(strPrefix);
}

int ConfigureManager::getInt(int nDefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return nDefault;
    return util::lexical_cast<int, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

float ConfigureManager::getFloat(float fDefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return fDefault;
    return util::lexical_cast<float, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

double ConfigureManager::getDouble(double dDefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return dDefault;
    return util::lexical_cast<double, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

long ConfigureManager::getLong(long lDefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return lDefault;
    return util::lexical_cast<long, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

Section ConfigureManager::getSection(cstring prefix) const {
    Section     s;
    std::string strPrefix = m_strPrefix + prefix;
    for (auto &val : m_mConfigKeyValMapper) {
        if (strncmp(val.first.c_str(), strPrefix.c_str(), prefix.size()) == 0) {
            s.insert(val);
        }
    }
    return s;
}

[[maybe_unused]] void ConfigureManager::changeAccessPath(cstring confPath) {
    m_strPrefix = confPath;
    if (m_strPrefix.back() != '/') {
        m_strPrefix.push_back('/');
    }
}
