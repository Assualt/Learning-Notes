//
// Created by 侯鑫 on 2023/12/26.
//

#ifndef SSP_TOOLS_LOG_IMPL_H
#define SSP_TOOLS_LOG_IMPL_H

#include <cstdint>

namespace ssp::base {
class LogImpl {
public:
    virtual void WriteData(const char *pData, uint32_t size);
    virtual ~LogImpl() = default;
};

class StdoutLog : public LogImpl {
public:
    void WriteData(const char *pData, uint32_t nSize) override;
};
}




#endif //SSP_TOOLS_LOG_IMPL_H
