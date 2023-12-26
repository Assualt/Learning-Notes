//
// Created by 侯鑫 on 2023/12/26.
//

#include "log_impl.h"
#include <iostream>

using namespace ssp::base;

void LogImpl::WriteData(const char *pData, uint32_t size)
{
}

void StdoutLog::WriteData(const char *pData, uint32_t size)
{
    std::cout.write(pData, static_cast<long>(size)).flush();
}