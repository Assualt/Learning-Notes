#pragma once
#include <cstdarg>
#include <cstring>
#include <memory>
#include <stdio.h>
namespace fsm {
class FLogger {
public:
    static void log(const char *fmt, ...) {
        int n = strlen(fmt) * 2;

        std::unique_ptr<char[]> fmtStr;
        va_list                 ap;
        while (true) {
            fmtStr.reset(new char[ n ]);
            (void)strcpy(&fmtStr[ 0 ], fmt);
            va_start(ap, fmt);
            auto final_n = vsnprintf(&fmtStr[ 0 ], n, fmt, ap);
            va_end(ap);
            if (final_n < 0 || final_n > n) {
                n += abs(final_n - n + 1);
            } else {
                break;
            }
        }

        printf("%s", fmtStr.get());
    }
};
} // namespace fsm
