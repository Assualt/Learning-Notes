#include "DirScanner.h"
#include "Logging.h"

using namespace muduo;
using namespace muduo::base;

void FileAttr::SetParentPath(const std::string &strParent, bool boExtendAllInfo) {
    m_strParent = strParent;
    if (strParent.back() != '/') {
        m_strParent.append("/");
    }
    if (!boExtendAllInfo) {
        return;
    }

    std::string strFullPath(m_strParent);
    strFullPath += m_strName;
    struct stat info;
    if (stat(strFullPath.c_str(), &info) == 0) {
        m_nFlags  = (info.st_mode & S_IFDIR) ? type_DIR : ((info.st_mode == S_IFLNK) ? type_Link : type_FILE);
        m_nSize   = info.st_size;
        m_tCreate = Timestamp::fromUnixTime(info.st_ctime, 0);
        m_tModify = Timestamp::fromUnixTime(info.st_mtime, 0);
        m_tRead   = Timestamp::fromUnixTime(info.st_atime, 0);
    }
}

DirScanner::DirScanner(const char *pathName)
    : m_nHandle(nullptr) {
    if (pathName != nullptr) {
        StartSearch(pathName);
    }
}

DirScanner::~DirScanner() { CloseHandle(); }

void DirScanner::CloseHandle() {
    if (m_nHandle) {
        closedir(m_nHandle);
        m_nHandle = nullptr;
    }
}

void DirScanner::StartSearch(const std::string &path) {
    CloseHandle();
    m_nHandle = opendir(path.c_str());
    if (m_nHandle == nullptr) {
        logger.warning("open %s dir failed", path);
    }
}

bool DirScanner::Fetch(FileAttr &attr) {
    if (m_nHandle == nullptr) {
        return false;
    }
    struct dirent *dp = readdir(m_nHandle);
    if (dp == nullptr) {
        return false;
    }
    attr.SetInfor(dp->d_name);
    return true;
}