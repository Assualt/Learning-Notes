#pragma once

#include <sys/prctl.h>

namespace muduo {
namespace base {
class System {
public:
    static inline char *GetCurrentThreadName();
};

inline char *System::GetCurrentThreadName() {
    char tempName[ 256 ] = {0};
    (void)prctl(PR_GET_NAME, tempName);
    return tempName;
}

} // namespace base
} // namespace muduo
