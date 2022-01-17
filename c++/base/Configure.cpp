#include "Configure.h"
using namespace muduo;
using namespace muduo::base;

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
        auto filetype = detail::getFileType(fileName);
        if (filetype == detail::Type_Dir) {
            GetAllFiles(fileName, suffix);
        } else if (filetype == detail::Type_File && detail::endWith(fileName, suffix)) {
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
        strPrefix.append(detail::trimRight(file.substr(pos + 1), m_Suffix));
    } else {
        strPrefix.append(detail::trimRight(file, m_Suffix));
    }
    std::string   strLine;
    std::ifstream fin(file);
    if (!fin.is_open()) {
        return;
    }

    std::string strSectionKey, strKey, strVal;
    bool        bFindSection = false;
    while (getline(fin, strLine)) {
        strLine = detail::trim(strLine, std::string(" "));
        if (strLine.empty() || detail::startWith(strLine, "#")) {
            continue;
        }

        size_t nPos = 0;
        if ((nPos = strLine.find("#")) != std::string::npos)
            strLine = strLine.substr(0, nPos);
        if (strLine.back() == ']' && strLine.front() == '[' && detail::count(strLine, '[') == 1 && detail::count(strLine, ']') == 1) {
            auto temp = strLine.substr(1, strLine.size() - 2);
            if (temp.empty()) {
                continue;
            }
            strSectionKey = strPrefix + "/" + temp;
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
                m_mConfigKeyValMapper[ strSectionKey + "/" + strKey ] = strVal;
            }
        }
    }
    fin.close();
}

bool ConfigureManager::getBool(bool bdefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return bdefault;
    if (strcasecmp(m_mConfigKeyValMapper.at(strPrefix).c_str(), "true") == 0)
        return true;
    else if (strcasecmp(m_mConfigKeyValMapper.at(strPrefix).c_str(), "false") == 0)
        return false;
    return detail::lexical_cast<bool, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

std::string ConfigureManager::getString(cstring strdefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return strdefault;
    return m_mConfigKeyValMapper.at(strPrefix);
}

int ConfigureManager::getInt(int ndefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return ndefault;
    return detail::lexical_cast<int, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

float ConfigureManager::getFloat(float fdefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return fdefault;
    return detail::lexical_cast<float, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

double ConfigureManager::getDouble(double ddefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return ddefault;
    return detail::lexical_cast<double, std::string>(m_mConfigKeyValMapper.at(strPrefix));
}

long ConfigureManager::getLong(long ldefault, cstring prefix) const {
    std::string strPrefix = m_strPrefix + prefix;
    if (m_mConfigKeyValMapper.find(strPrefix) == m_mConfigKeyValMapper.end())
        return ldefault;
    return detail::lexical_cast<long, std::string>(m_mConfigKeyValMapper.at(strPrefix));
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

void ConfigureManager::changeAccessPath(cstring confpath) {
    m_strPrefix = confpath;
}
