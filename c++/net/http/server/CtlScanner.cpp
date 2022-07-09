#include "CtlScanner.h"
#include "base/DirScanner.h"
#include "base/Logging.h"
#include "net/http/HttpParttern.h"
#include "net/http/HttpUtils.h"
#include "net/http/server/HttpServer.h"
#include <algorithm>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
using namespace muduo::base;

using ControllerEntry = int (*)(std::string *, int *, bool *, uintptr_t *);

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
                // TODO so变化之后，需要重新load一把才可以完成功能
            } else {
                continue;
            }

            auto helper = std::make_unique<DllHelper>();
            if (helper->open(attr.GetFullName().c_str(), DllHelper::LOAD_LAZY) != 0) {
                logger.warning("%s so is invalid, msg:%s", attr.GetFullName(), helper->errMsg());
                continue;
            }

            std::string     symbolName = attr.BriefName() + "_Entry";
            ControllerEntry entry;
            entry = reinterpret_cast<ControllerEntry>(helper->GetSymbol(symbolName.c_str()));
            if (entry == nullptr) {
                logger.warning("%s so is invalid, msg:%s", attr.GetFullName(), helper->errMsg());
                continue;
            }
            std::string key;
            int         method;
            bool        needval;
            uintptr_t   obj;
            auto        ret = entry(&key, &method, &needval, &obj);
            HttpServer::getMapper().addRequestObject({key, method, needval}, obj);
            logger.info("success reg call func name:%s pattern:%s, method:%d, needval:%b obj:%x", attr.BriefName(), key, method, needval, obj);
            dllmaps_.push_back({attr.BriefName(), std::move(helper)});
        }
        scanner.CloseHandle();
        sleep(5);
    }
}