#include "conf/xconfmgrbase.h"

NAMESPACE_BEGIN

TConfigureManagerBase::TConfigureManagerBase(const xmt::tstring &strConfDir, CONF_TYPE nType) :
        m_strConfigureDir(strConfDir),
        m_nConfType(nType) {
    ScanFilePath(m_strConfigureDir);
}

bool TConfigureManagerBase::ChangeAccessPath(const tstring &strConfDir) {
    m_strConfigureDir = strConfDir;
    m_strFileNames.clear();
    ScanFilePath(m_strConfigureDir);
    return true;
}

tstring TConfigureManagerBase::getConfType() {
    int type = static_cast<int>(m_nConfType);
    tstring strType;
    switch (type) {
    case TYPE_UNKNOWN:
        strType = "";
        break;
    case TYPE_INI:
        strType = ".ini";
        break;
    case TYPE_XML:
        strType = ".xml";
        break;
    case TYPE_JSON:
        strType = ".json";
        break;
    case TYPE_YML:
        strType = ".yml";
        break;
    default:
        strType = ".ini";
        break;
    }
    return strType;
}

const std::vector<tstring> &TConfigureManagerBase::getAllConfFiles() const {
    return m_strFileNames;
}

void TConfigureManagerBase::ScanFilePath(const xmt::tstring &strDirName) {
    if (strDirName.empty())
        return;
    struct stat st;
    if (stat(strDirName.c_str(), &st) == -1)
        throw new TConfigureException(
                TFmtString("config dir is not found. stat error.msg:%").arg(strerror(errno)).str(),
                XException::XEP_ERROR);
    DIR *dir;
    if ((dir = opendir(m_strConfigureDir.c_str())) != NULL) {
        struct dirent *cur_dirent;
        while ((cur_dirent = readdir(dir)) != NULL) {
            if (!strcmp(cur_dirent->d_name, ".") || !strcmp(cur_dirent->d_name, ".."))
                continue;
            tstring strTmpFile = TFmtString("%/%").arg(strDirName).arg(cur_dirent->d_name).str();
            struct stat tmp;
            stat(strTmpFile.c_str(), &tmp);
            switch (tmp.st_mode & S_IFMT) {
            case S_IFREG:
                if (TStringHelper::endWith(strTmpFile, getConfType().c_str()))
                    m_strFileNames.push_back(strTmpFile);
                break;
            case S_IFDIR:
                ScanFilePath(strTmpFile);
                break;
            default:
                break;
            }
        }
    }
    closedir(dir);
}

TInIConfigureManager::TInIConfigureManager(const tstring &strConfDir) : TConfigureManagerBase(strConfDir, TYPE_INI) {}

bool TInIConfigureManager::init() {
    return initIniFiles();
}

bool TInIConfigureManager::initIniFiles() {
    std::vector<tstring> fileNames = TConfigureManagerBase::getAllConfFiles();
    for (auto it : fileNames) {
        TFileListMap tmp;
        initSingerFile(tmp, it);
        if (m_mKVMap.find(it) == m_mKVMap.end()) {
            // it-> /a/b/c/d/e/f/x.cf
            // base->/a/b/c/d/e/f/
            // key->x
            tstring strKey;
            if (it.find(m_strConfigureDir) != tstring::npos) {
                strKey = it.substr(it.find(m_strConfigureDir) + m_strConfigureDir.size());
            }
            // remove suffix
            strKey = strKey.substr(1, strKey.rfind(getConfType()) - 1);
            m_mKVMap[strKey] = tmp;
        }
    }
    return true;
}

void TInIConfigureManager::initSingerFile(TFileListMap &t, const tstring &confPath) {
    try {
        if (confPath.empty())
            throw TConfigureException(TFmtString("Input ConfPath is empty").str());
        std::ifstream fin(confPath);
        if (!fin.is_open()) {
            throw TConfigureException(TFmtString("Can't Open % file conf path.").arg(confPath).str());
        }
        tstring lineTmp;
        tstring strSectionKey;
        std::map<tstring, tstring> SectionKvMap;
        while (getline(fin, lineTmp)) {
            lineTmp = TStringHelper::trim(lineTmp);
            if (lineTmp.empty() || TStringHelper::startWith(lineTmp, "#") || TStringHelper::startWith(lineTmp, "//"))
                continue;
            // found section
            if (TStringHelper::startWith(lineTmp, "[")) {
                size_t nPos = lineTmp.find("]");
                if (nPos != tstring::npos && nPos != 1) {  // Found New Keys
                    if (!strSectionKey.empty()) {
                        if (t.find(strSectionKey) != t.end()) {
                            std::cout << TFmtString(
                                                 "duplicate Section key(%) has "
                                                 "been found")
                                                 .arg(strSectionKey)
                                                 .str()
                                      << std::endl;
                        }
                        t[strSectionKey] = SectionKvMap;
                        SectionKvMap.clear();
                    }
                    strSectionKey = TStringHelper::trim(lineTmp.substr(1, nPos - 1));
                }
            } else if (!strSectionKey.empty()) {  // found key=val
                size_t nPos = lineTmp.find("=");
                tstring strKey, strVal;
                if (nPos != tstring::npos) {  // found =
                    strKey = TStringHelper::trim(lineTmp.substr(0, nPos));
                    // next to found val
                    strVal = TStringHelper::trim(lineTmp.substr(nPos + 1));
                    if (strVal.find("#") != tstring::npos) {  //发现有注释需要剔除掉注释部分
                        strVal = strVal.substr(0, strVal.find("#"));
                    }
                    if (strKey.empty() || strVal.empty())
                        continue;
                    if (TStringHelper::startWith(strVal, "\"") && TStringHelper::endWith(strVal, "\"")) {
                        strVal.erase(0);
                        strVal.erase(strVal.size() - 1);
                    }
                    SectionKvMap[strKey] = strVal;
                }
            }
        }

        if (t.find(strSectionKey) != t.end()) {
            std::cout << TFmtString("duplicate key(%) has been found").arg(strSectionKey).str() << std::endl;
        }
        t[strSectionKey] = SectionKvMap;
        fin.close();
    } catch (TConfigureException &e) {
        std::cout << "Catch Configuration Exception: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cout << "Catch exception: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Catch unknown exception: " << std::endl;
    }
}

int TInIConfigureManager::getInt(int nDefault, const tstring &strPrefix, const tstring &strSuffix) {
    bool isDeault;
    tstring strVal = getBaseString("", strPrefix, strSuffix, isDeault);
    if (isDeault)
        return nDefault;
    return TStringHelper::toInt(strVal);
}
bool TInIConfigureManager::getBool(bool nDefault, const tstring &strPrefix, const tstring &strSuffix) {
    bool isDeault;
    tstring strVal = getBaseString("", strPrefix, strSuffix, isDeault);
    if (isDeault)
        return nDefault;
    if (TStringHelper::strncmp(strVal.c_str(), "true"))
        return true;
    else if (TStringHelper::strncmp(strVal.c_str(), "false"))
        return false;
    int n = TStringHelper::toInt(strVal);
    if (n >= 1)
        return true;
    return false;
}
float TInIConfigureManager::getFloat(float nDefault, const tstring &strPrefix, const tstring &strSuffix) {
    bool isDeault;
    tstring strVal = getBaseString("", strPrefix, strSuffix, isDeault);
    if (isDeault)
        return nDefault;
    return TStringHelper::toFloat(strVal);
}
double TInIConfigureManager::getDouble(double nDefault, const tstring &strPrefix, const tstring &strSuffix) {
    bool isDeault;
    tstring strVal = getBaseString("", strPrefix, strSuffix, isDeault);
    if (isDeault)
        return nDefault;
    return TStringHelper::toDouble(strVal);
}
tstring TInIConfigureManager::getString(tstring nDefault, const tstring &strPrefix, const tstring &strSuffix) {
    bool isDefault;
    return getBaseString(nDefault, strPrefix, strSuffix, isDefault);
}
tstring TInIConfigureManager::getBaseString(
        tstring nDefault,
        const tstring &strPrefix,
        const tstring &strSuffix,
        bool &isDefault) {
    isDefault = true;
    if (m_mKVMap.find(strPrefix) == m_mKVMap.end()) {
        return nDefault;
    }
    size_t nPos;
    if ((nPos = strSuffix.rfind("/")) == tstring::npos) {
        return nDefault;
    }
    tstring strSection = strSuffix.substr(0, nPos);
    tstring strKey = strSuffix.substr(nPos + 1);
    TFileListMap fileMap = m_mKVMap[strPrefix];
    if (fileMap.find(strSection) == fileMap.end()) {
        return nDefault;
    }
    std::map<tstring, tstring> kvMap = fileMap[strSection];
    if (kvMap.find(strKey) == kvMap.end()) {
        return nDefault;
    }
    isDefault = false;
    return kvMap[strKey];
}

NAMESPACE_END
