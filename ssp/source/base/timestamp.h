//
// Created by 侯鑫 on 2023/12/26.
//

#ifndef SSP_TOOLS_TIMESTAMP_H
#define SSP_TOOLS_TIMESTAMP_H

#include "object.h"
#include <cstdint>
#include <sys/time.h>
#include "exception.h"

namespace ssp::base {
DECLARE_EXCEPTION(TimeStampException, Exception)
class TimeStamp : public  Object{
public:
    TimeStamp();

    explicit TimeStamp(uint64_t val);

    [[nodiscard]] const char * GetObjectName() const override
    {
        return "TimeStamp";
    }

public:
    std::string ToFmtString(const char *fmt = "%Y-%m-%d %H:%M:%S.000.%Z%z") const;

    [[maybe_unused]] [[nodiscard]] uint64_t MicroSeconds() const;

    [[nodiscard]] time_t Seconds() const;

    [[nodiscard]] uint64_t MSeconds() const;

public:
    static TimeStamp Now();

private:
    uint64_t msSeconds_{0};
};
}

#endif //SSP_TOOLS_TIMESTAMP_H
