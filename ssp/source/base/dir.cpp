//
// Created by 侯鑫 on 2024/1/1.
//

#include "dir.h"
#include "log.h"

using namespace ssp::base;

DirScanner::DirScanner(const char *pathName)
    : handle_(nullptr)
{
    if (pathName != nullptr) {
        StartSearch(pathName);
    }
}

DirScanner::~DirScanner()
{
    CloseHandle();
}

void DirScanner::CloseHandle()
{
    if (handle_) {
        closedir(handle_);
        handle_ = nullptr;
    }
}

void DirScanner::StartSearch(const std::string &path)
{
    CloseHandle();
    handle_ = opendir(path.c_str());
    rootPath_ = path;
    if (handle_ == nullptr) {
        log_sys.Warning("open %s dir failed. errno:%d", path, errno);
    }
}

bool DirScanner::Fetch(FileAttr &attr)
{
    if (handle_ == nullptr) {
        return false;
    }
    struct dirent *dp = readdir(handle_);
    if (dp == nullptr) {
        return false;
    }

    File newFile(dp->d_name, FileOpType::READ);
    attr = newFile.GetFileAttr();
    attr.name = std::string(dp->d_name, dp->d_namlen);
    attr.absPath = rootPath_ + "/" + attr.name;
    newFile.Close();
    return true;
}

void DirScanner::Dump(std::ostream &os)
{
    os << ">>> DirScanner:";
}