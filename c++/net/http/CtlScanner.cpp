#include "CtlScanner.h"
#include "HttpUtils.h"
#include "base/DirScanner.h"
#include "base/Dll.h"
#include "base/Logging.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
using namespace muduo::base;

typedef void (*ControllerEntry)();

void ControllerScanner::init(const std::string &path) {
    libsPaths_ = path;
}

void ControllerScanner::startTask() {
    auto func = [ this ]() { this->TaskCallback(); };
    thread_   = std::make_shared<Thread>(func, "ScannerTask");
    if (thread_ != nullptr) {
        thread_->Start();
    }
}

void ControllerScanner::stopTask() {
    if (thread_ != nullptr) {
        exit_ = true;
        thread_->Detached();
    }
}

void ControllerScanner::TaskCallback() {
    while (!exit_) {
        std::map<std::string, int> tmpMaps;
        DirScanner                 scanner(libsPaths_.c_str());
        FileAttr                   attr;

        while (scanner.Fetch(attr)) {
            if (attr.IsDir() || attr.IsLink()) {
                logger.debug("attr name %s skip, isdir:%b isLink:%b", attr.GetFullName(), attr.IsDir(), attr.IsLink());
                continue;
            }

            if (!utils::endWith(attr.GetFullName(), ".so")) {
                logger.debug("no interest in %s", attr.GetFullName());
                continue;
            }

            attr.SetParentPath(libsPaths_);
            auto iter = libmaps_.find(attr.GetName());
            if (iter != libmaps_.end() && iter->second == attr.GetSize()) {
                logger.debug("file name %s is equals, skip", attr.GetName());
                continue;
            }

            if (iter == libmaps_.end()) {
                logger.info("%s should add here,new size:%d", attr.GetName(), attr.GetSize());
                libmaps_.emplace(attr.GetName(), attr.GetSize());
            } else if (iter->second != attr.GetSize()) {
                logger.info("%s size is not equals, old size:%d new size:%d", attr.GetName(), libmaps_[ attr.GetName() ], attr.GetSize());
                libmaps_.emplace(attr.GetName(), attr.GetSize());
            } else {
                continue;
            }

            DllHelper helper;
            if (helper.open(attr.GetFullName().c_str(), DllHelper::LOAD_LAZY) != 0) {
                logger.warning("%s so is invalid, msg:%s", attr.GetFullName(), helper.errMsg());
                continue;
            }

            std::string symbolName = attr.BriefName() + "_Entry";
            int (*entry)(void);
            entry = reinterpret_cast<int (*)(void)>(helper.GetSymbol(symbolName.c_str()));
            if (entry == nullptr) {
                logger.warning("%s so is invalid, msg:%s", attr.GetFullName(), helper.errMsg());
                continue;
            }
            auto ret = entry();
            logger.info("success reg call func name:%s ret:%d", attr.BriefName(), ret);
        }
        scanner.CloseHandle();
        sleep(5);
    }
}