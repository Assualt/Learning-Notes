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
                continue;
            }

            if (!utils::endWith(attr.GetFullName(), ".so")) {
                logger.debug("no interest in %s", attr.GetFullName());
                continue;
            }

            attr.SetParentPath(libsPaths_);

            if (libmaps_.count(attr.GetName()) && libmaps_[ attr.GetName() ] == attr.GetSize()) {
                logger.info("so %s is equals", attr.GetName());
                continue;
            }

            if (libmaps_.find(attr.GetName()) == libmaps_.end()) {
                logger.info("so %s is exist, so size:%d", attr.GetName(), attr.GetSize());
                tmpMaps.emplace(attr.GetName(), attr.GetSize());
            }

            if (libmaps_.count(attr.GetName()) && libmaps_[ attr.GetName() ] != attr.GetSize()) {
                logger.info("so %s is not equals, old size:%d new size:%d", attr.GetName(), libmaps_[ attr.GetName() ], attr.GetSize());
                tmpMaps.emplace(attr.GetName(), attr.GetSize());
            }

            DllHelper helper;
            if (helper.open(attr.GetFullName().c_str(), DllHelper::LOAD_LAZY) != 0) {
                logger.warning("%s so is invalid, msg:%s", attr.GetFullName(), helper.errMsg());
                continue;
            }

            std::string symbolName = attr.BriefName() + "_Entry";
            int (*entry)(void);
            entry = reinterpret_cast<int(*)(void)>(helper.GetSymbol(symbolName.c_str()));
            if (entry == nullptr) {
                logger.warning("%s so is invalid, msg:%s", attr.GetFullName(), helper.errMsg());
                continue;
            }
            auto ret = entry();
            logger.info("success reg call func name:%s ret:%d", attr.BriefName(), ret);
        }
        libmaps_.swap(tmpMaps);
        scanner.CloseHandle();
        sleep(10);
    }
}