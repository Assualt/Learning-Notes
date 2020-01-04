#include "conf/xconfmgrbase.h"

NAMESPACE_BEGIN
TConfigureManagerBase::TConfigureManagerBase(const xmt::tstring &strConfDir) :
        m_strConfigureDir(strConfDir),
        m_nConfType(TYPE_INI) {
    ScanFilePath(m_strConfigureDir);
}

TConfigureManagerBase::TConfigureManagerBase(
        const xmt::tstring &strConfDir,
        CONF_TYPE nType) :
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
                TFmtstring("config dir is not found. stat error.msg:%")
                        .arg(strerror(errno))
                        .c_str(),
                XException::XEP_ERROR);
    DIR *dir;
    if ((dir = opendir(m_strConfigureDir.c_str())) != NULL) {
        struct dirent *cur_dirent;
        while ((cur_dirent = readdir(dir)) != NULL) {
            if (!strcmp(cur_dirent->d_name, ".") ||
                !strcmp(cur_dirent->d_name, ".."))
                continue;
            tstring strTmpFile = TFmtstring("%/%")
                                         .arg(strDirName)
                                         .arg(cur_dirent->d_name)
                                         .c_str();
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

TInIConfigureManager::TInIConfigureManager(const tstring &strConfDir) :
        TConfigureManagerBase(strConfDir) {}

bool TInIConfigureManager::init() {
    return initIniFiles();
}

bool TInIConfigureManager::initIniFiles() {
    std::vector<tstring> fileNames = TConfigureManagerBase::getAllConfFiles();
    for (auto it : fileNames) {
        TFileListMap tmp;
        initSingerFile(tmp, it);
        if (m_mKVMap.find(it) == m_mKVMap.end()) {
            m_mKVMap[it] = tmp;
        }
    }
    return true;
}

void TInIConfigureManager::initSingerFile(
        TFileListMap &t,
        const tstring &confPath) {
    try {
        if (confPath.empty())
            throw TConfigureException(
                    TFmtstring("Input ConfPath is empty").c_str());
        std::ifstream fin(confPath);
        if (!fin.is_open()) {
            throw TConfigureException(TFmtstring("Can't Open % file conf path.")
                                              .arg(confPath)
                                              .c_str());
        }
        tstring lineTmp;
        while (getline(fin, lineTmp)) {
            if (lineTmp.empty())
                continue;
            TStringHelper::trim(lineTmp);
            if (!TStringHelper::startWith(lineTmp, "#")) {
                // Search Section First
                tstring strSectionName;
                size_t nPos;
                if ((nPos = lineTmp.find("[")) != tstring::npos) {
                    size_t nEnd = lineTmp.find("]" + nPos + 1);
                    if (nEnd != tstring::npos) {
                        if (nEnd == nPos + 1)
                            continue;
                        strSectionName =
                                lineTmp.substr(nPos + 1, nEnd - nPos - 1);
                        // Search Key Val
                        size_t nSeekPos;  // 记录
                        std::map<tstring, tstring> tmpMap;
                        tstring tmpLine;
                        while (1) {
                            nSeekPos = fin.tellg();
                            if (getline(fin, tmpLine)) {
                                TStringHelper::trim(tmpLine);
                                if (tmpLine.empty())
                                    continue;
                                size_t nBegin;
                                if ((nBegin = tmpLine.find("=")) !=
                                    tstring::npos) {
                                    tstring strKey = tmpLine.substr(0, nBegin);
                                    tstring strLeftVal =
                                            tmpLine.substr(nBegin + 1);
                                    TStringHelper::trim(strLeftVal);
                                    if (TStringHelper::startWith(
                                                strLeftVal, "\"")) {
                                        size_t b = strLeftVal.find("\"", 1);
                                        if (b != tstring::npos) {
                                            tstring strVal =
                                                    strLeftVal.substr(1, b - 1);
                                            TStringHelper::trim(strVal);
                                            if (tmpMap.find(strKey) ==
                                                tmpMap.end())
                                                tmpMap[strKey] = strVal;
                                        }
                                    } else {
                                        if (strLeftVal.find("#") !=
                                            tstring::npos) {
                                            tstring strVal = strLeftVal.substr(
                                                    0, strLeftVal.find("#"));
                                            TStringHelper::trim(strVal);
                                            if (tmpMap.find(strKey) ==
                                                tmpMap.end())
                                                tmpMap[strKey] = strVal;
                                        }
                                    }
                                }
                                if (TStringHelper::startWith(
                                            tmpLine, "[")) {  // find Next Key
                                    if (t.find(strSectionName) == t.end()) {
                                        t[strSectionName] = tmpMap;
                                        fin.seekg(nSeekPos);
                                        break;
                                    }
                                }
                            } else {
                                if (t.find(strSectionName) == t.end()) {
                                    t[strSectionName] = tmpMap;
                                }
                                break;
                            }
                        }
                    } else
                        continue;
                }
            }
        }
        fin.close();

    } catch (std::exception &e) {
        std::cout << "Catch exception: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Catch unknown exception: " << std::endl;
    }
}

NAMESPACE_END
