//
// Created by 侯鑫 on 2024/1/1.
//

#include "config.h"
#include "string_utils.h"
#include "dir.h"
#include "format.h"
#include <fstream>

using namespace ssp::base;

ConfigureManager::ConfigureManager(ssp::base::cstring confPath, ssp::base::cstring suffix)
    :suffix_(suffix)
{
    GetAllFiles(confPath, suffix);
}

void ConfigureManager::GetAllFiles(cstring path, cstring suffix) 
{
    DirScanner scanner(path.c_str());
    FileAttr attr;
    while (scanner.Fetch(attr)) {
        if (attr.name == "." || attr.name == "..") {
            continue;
        }

        if (!util::EndsWithIgnoreCase(attr.name, suffix)) {
            continue;
        }

        allConfFiles_.emplace_back(attr.absPath);
    }
}

void ConfigureManager::Init() 
{
    for (const auto &file : allConfFiles_)
    {
        InitFile(file);
    }
}

void ConfigureManager::InitFile(cstring file)
{
    std::string strPrefix = "/";
    size_t      pos       = file.rfind('/');
    if (pos != std::string::npos) {
        strPrefix.append(util::TrimRight(file.substr(pos + 1), suffix_));
    } else {
        strPrefix.append(util::TrimRight(file, suffix_));
    }
    std::string   strLine;
    std::ifstream fin(file);
    if (!fin.is_open()) {
        return;
    }

    std::string strSectionKey, strKey, strVal;
    bool        bFindSection = false;
    while (std::getline(fin, strLine)) {
        strLine = util::trim(strLine, ' ');
        if (strLine.empty() || util::StartsWith(strLine, "#")) {
            continue;
        }

        size_t nPos;
        if ((nPos = strLine.find('#')) != std::string::npos)
            strLine = strLine.substr(0, nPos);
        if (strLine.back() == ']' && strLine.front() == '[' && util::Count(strLine, '[') == 1 &&
            util::Count(strLine, ']') == 1) {
            auto temp = strLine.substr(1, strLine.size() - 2);
            if (temp.empty()) {
                continue;
            }
            strSectionKey = FmtString("%/%").arg(strPrefix).arg(temp).str();
            bFindSection  = true;
        } else if (bFindSection && util::Count(strLine, '=')) {
            auto pos1 = strLine.find('=');
            strKey    = util::trim(strLine.substr(0, pos1), ' ');
            strVal    = util::trim(strLine.substr(pos1 + 1), ' ');
            strKey    = util::Trim(strKey, std::string("'"));
            strVal    = util::Trim(strVal, std::string("'"));
            strKey    = util::Trim(strKey, std::string("\""));
            strVal    = util::Trim(strVal, std::string("\""));
            if (!strKey.empty() && !strVal.empty()) {
                configKeyValMapper_[ FmtString("%/%").arg(strSectionKey).arg(strKey).str() ] = strVal;
            }
        }
    }
    fin.close();
}

bool ConfigureManager::GetBool(bool bDefault, cstring prefix) const
{
    std::string strPrefix = prefix_ + prefix;
    if (configKeyValMapper_.find(strPrefix) == configKeyValMapper_.end())
        return bDefault;
    if (strcasecmp(configKeyValMapper_.at(strPrefix).c_str(), "true") == 0)
        return true;
    else if (strcasecmp(configKeyValMapper_.at(strPrefix).c_str(), "false") == 0)
        return false;
    return lexical_cast<bool, std::string>(configKeyValMapper_.at(strPrefix));
}

[[maybe_unused]] std::string ConfigureManager::GetString(cstring strDefault, cstring prefix) const
{
    std::string strPrefix = prefix_ + prefix;
    if (configKeyValMapper_.find(strPrefix) == configKeyValMapper_.end())
        return strDefault;
    return configKeyValMapper_.at(strPrefix);
}

int ConfigureManager::GetInt(int nDefault, cstring prefix) const
{
    std::string strPrefix = prefix_ + prefix;
    if (configKeyValMapper_.find(strPrefix) == configKeyValMapper_.end())
        return nDefault;
    return lexical_cast<int, std::string>(configKeyValMapper_.at(strPrefix));
}

float ConfigureManager::GetFloat(float fDefault, cstring prefix) const
{
    std::string strPrefix = prefix_ + prefix;
    if (configKeyValMapper_.find(strPrefix) == configKeyValMapper_.end())
        return fDefault;
    return lexical_cast<float, std::string>(configKeyValMapper_.at(strPrefix));
}

double ConfigureManager::GetDouble(double dDefault, cstring prefix) const
{
    std::string strPrefix = prefix_ + prefix;
    if (configKeyValMapper_.find(strPrefix) == configKeyValMapper_.end())
        return dDefault;
    return lexical_cast<double, std::string>(configKeyValMapper_.at(strPrefix));
}

long ConfigureManager::GetLong(long lDefault, cstring prefix) const
{
    std::string strPrefix = prefix_ + prefix;
    if (configKeyValMapper_.find(strPrefix) == configKeyValMapper_.end())
        return lDefault;
    return lexical_cast<long, std::string>(configKeyValMapper_.at(strPrefix));
}

Section ConfigureManager::GetSection(cstring prefix) const
{
    Section     s;
    std::string strPrefix = prefix_ + prefix;
    for (auto &val : configKeyValMapper_) {
        if (strncmp(val.first.c_str(), strPrefix.c_str(), strPrefix.size()) != 0) {
            continue;
        }

        if (val.first.size() < strPrefix.size()) {
            continue;
        }

        auto rVal = val.first.substr(strPrefix.size());
        if (util::Count(rVal, '/') == 0) {
            s.insert(val);
        }
    }
    return s;
}

[[maybe_unused]] void ConfigureManager::ChangeAccessPath(cstring confPath)
{
    prefix_ = confPath;
    if (prefix_.back() != '/') {
        prefix_.push_back('/');
    }
}

void ConfigureManager::Dump(std::ostream &os)
{
    os << ">>> Dump Configuration:" << std::endl;
    os << "  >> Files:" << allConfFiles_.size() << std::endl;
    uint32_t idx = 0;
    std::for_each(allConfFiles_.begin(), allConfFiles_.end(), [&os, &idx](auto item) {
        os << "     [" << ToFixedString(idx++, 3, false)  << "] ==> " << item << std::endl;
    });
    os << "  >> keyMapper:" << configKeyValMapper_.size() << std::endl;
    idx = 0;
    std::for_each(configKeyValMapper_.begin(), configKeyValMapper_.end(), [&os, &idx](auto item) {
        os << "     [" << ToFixedString(idx++, 3, false) << "] ==> " << item.first << " ==> " << item.second << std::endl;
    });
}
