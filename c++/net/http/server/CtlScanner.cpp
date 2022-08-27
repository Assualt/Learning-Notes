#include "CtlScanner.h"
#include "base/Logging.h"
#include "net/http/HttpParttern.h"
#include "net/http/HttpUtils.h"
#include "net/http/server/HttpServer.h"
#include <algorithm>
#include <unistd.h>
using namespace muduo::base;

using ControllerEntry = int (*)(std::string *, int *, bool *, bool *, uintptr_t *);

void ControllerScanner::init(const std::string &path) { libsPaths_ = path; }

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

bool ControllerScanner::needUpdate(const FileAttr &attr) {
    auto itr = std::find_if(dllVectors_.begin(), dllVectors_.end(),
                            [ &attr ](auto info) { return info.fullPath_ == attr.GetFullName(); });

    if (itr == dllVectors_.end()) {
        return true;
    }

    return itr->soModifyTime_ < attr.GetModifyTime();
}

void ControllerScanner::update(const FileAttr &attr) {
    auto itr = std::find_if(dllVectors_.begin(), dllVectors_.end(),
                            [ &attr ](auto info) { return info.fullPath_ == attr.GetFullName(); });

    if (itr != dllVectors_.end()) {
        unRegisterHandle(*itr);
        dllVectors_.erase(itr);
        logger.info("unregister pattern for %s", itr->pattern_);
    }

    DllInfo info;
    info.soModifyTime_ = attr.GetModifyTime();
    info.fullPath_     = attr.GetFullName();
    info.soName_       = attr.GetName();
    info.helper_       = std::make_shared<DllHelper>();

    if (info.helper_ == nullptr) {
        logger.warning("dll helper is nullptr. skip add");
        return;
    }

    auto   entryName = attr.GetName();
    size_t pos;
    if ((pos = entryName.find(".")) != string::npos) {
        entryName = entryName.substr(0, pos);
    }

    if (info.helper_->open(info.fullPath_.c_str(), DllHelper::LOAD_LAZY) != 0) {
        logger.info("open so for %s failed.", info.fullPath_);
        return;
    }

    entryName             = FmtString("%_Entry").arg(entryName).str();
    ControllerEntry entry = reinterpret_cast<ControllerEntry>(info.helper_->GetSymbol(entryName.c_str()));
    if (entry == nullptr) {
        logger.warning("%s so is invalid, msg:%s, entryName:%s", attr.GetFullName(), info.helper_->errMsg(), entryName);
        return;
    }

    bool      needVal;
    uintptr_t obj;
    int       method;
    bool      useRegex;
    auto      ret = entry(&info.pattern_, &method, &needVal, &useRegex, &obj);
    HttpServer::getMapper().addRequestObject({info.pattern_, method, needVal, useRegex}, obj);
    logger.info("success reg call func name:%s pattern:%s, method:%d, needVal:%b useRegex:%b obj:%x ret:%d",
                attr.BriefName(), info.pattern_, method, needVal, useRegex, obj, ret);

    dllVectors_.push_back(std::move(info));
}

void ControllerScanner::unRegisterHandle(DllInfo &info) {
    HttpServer::getMapper().removeRequestObject(info.pattern_);
    info.helper_->close();
}

void ControllerScanner::TaskCallback() {
    while (!exit_) {
        DirScanner scanner(libsPaths_.c_str());
        FileAttr   attr;
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
            if (!needUpdate(attr)) {
                continue;
            }

            update(attr);
        }
        scanner.CloseHandle();
        sleep(5);
    }
}