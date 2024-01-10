//
// Created by 侯鑫 on 2023/12/26.
//

#ifndef SSP_TOOLS_LOG_IMPL_H
#define SSP_TOOLS_LOG_IMPL_H

#include <cstdint>
#include <functional>
#include <fstream>
#include <utility>
#include "format.h"

namespace ssp::base {

class LogImpl {
public:
    virtual void WriteData(const char *pData, uint32_t size);
    virtual ~LogImpl() = default;
    virtual bool ChangeAccess() { return false; }
};

class StdoutLog : public LogImpl {
public:
    void WriteData(const char *pData, uint32_t nSize) override;
};

class RollingFileLog : public LogImpl {
public:
    explicit RollingFileLog(std::string filePath, std::string prefix, std::string postfix = "%Y-%m-%d")
        : LogImpl(), filePath_(std::move(filePath)), prefix_(std::move(prefix)), postfix_(std::move(postfix))
    {
    }

    ~RollingFileLog() override;

    void WriteData(const char *data, uint32_t size) override;

    bool ChangeAccess() override;

private:
    std::string filePath_;
    std::string prefix_;
    std::string postfix_;

    std::string curFile_;
    std::ofstream fout_;
};



}




#endif //SSP_TOOLS_LOG_IMPL_H
