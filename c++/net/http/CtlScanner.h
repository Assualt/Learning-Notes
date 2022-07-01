#pragma once

#include "base/Dll.h"
#include "base/Thread.h"
#include <iostream>
#include <map>
using namespace muduo::base;

class ControllerScanner {
public:
    void init(const std::string &libPaths);
    void startTask();
    void stopTask();

    void TaskCallback();

private:
    std::string                                                     libsPaths_;
    std::shared_ptr<Thread>                                         thread_{nullptr};
    bool                                                            exit_{false};
    std::map<std::string, int>                                      libmaps_;
    std::vector<std::pair<std::string, std::unique_ptr<DllHelper>>> dllmaps_;
};