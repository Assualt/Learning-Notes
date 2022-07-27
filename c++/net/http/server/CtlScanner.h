#pragma once

#include "base/DirScanner.h"
#include "base/Dll.h"
#include "base/Thread.h"
#include "base/Timestamp.h"
#include <iostream>
#include <map>
using namespace muduo::base;

class ControllerScanner {
    struct DllInfo {
        std::string                pattern_;
        std::string                fullPath_;
        std::string                soName_;
        Timestamp                  soModifyTime_;
        std::shared_ptr<DllHelper> helper_{nullptr};
    };

    using DllVectors = std::vector<DllInfo>;

public:
    void init(const std::string &libPaths);
    void startTask();
    void stopTask();

    void TaskCallback();

private:
    // 判断so是否
    bool needUpdate(const FileAttr &attr);

    void update(const FileAttr &attr);

    void unRegisterHandle(DllInfo &info);

private:
    std::string                libsPaths_;
    std::shared_ptr<Thread>    thread_{nullptr};
    bool                       exit_{false};
    std::map<std::string, int> libmaps_;
    DllVectors                 dllVectors_;
};