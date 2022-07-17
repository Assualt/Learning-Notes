#pragma once
#include <stdio.h>
namespace fsm {

class FLogger {
public:
    static void log(const char *fmt, ...) {
        printf(fmt);
    }
};
} // namespace fsm
