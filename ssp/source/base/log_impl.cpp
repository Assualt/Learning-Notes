//
// Created by 侯鑫 on 2023/12/26.
//

#include "log_impl.h"
#include "timestamp.h"
#include <iostream>

using namespace ssp::base;

void LogImpl::WriteData(const char *pData, uint32_t size)
{
}

void StdoutLog::WriteData(const char *pData, uint32_t size)
{
    std::cout.write(pData, static_cast<long>(size)).flush();
}

void RollingFileLog::WriteData(const char *data, uint32_t size)
{
    ChangeAccess();
    if (!fout_.is_open()) {
        return;
    }
    fout_.write(data, size).flush();
}

bool RollingFileLog::ChangeAccess()
{
    auto name = FmtString("%/%.log.%").arg(filePath_).arg(prefix_).arg(TimeStamp::Now().ToFmtString(postfix_.c_str())).str();
    if (curFile_.empty()) {
        curFile_ = name;
        fout_.open(curFile_.c_str(), std::ios_base::app | std::ios_base::out);
        return true;
    }

    if (curFile_ == name) {
        return false;
    }

    fout_.flush();
    fout_.close();
    fout_.open(curFile_.c_str(), std::ios::app | std::ios::out);
    return true;
}

RollingFileLog::~RollingFileLog()
{
    fout_.flush();
    fout_.close();
}
