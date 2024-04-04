//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_DIR_H
#define SSP_TOOLS_DIR_H

#include "file.h"
#include <dirent.h>
#include <sys/stat.h>
#include "object.h"

namespace ssp::base {

class DirScanner : Object {
public:
    explicit DirScanner(const char *pathName = nullptr);

    ~DirScanner() override;

public:
    void CloseHandle();

    bool Fetch(FileAttr &attr);

    void StartSearch(const std::string &strPath);

    void Dump(std::ostream &os) override;

protected:
    ::DIR *handle_ { nullptr };
    std::string rootPath_;
};

}

#endif //SSP_TOOLS_DIR_H
