#pragma once
#include <dlfcn.h>

namespace muduo::base {
class DllHelper {
public:
    enum LOAD_TYPE { LOAD_LAZY = RTLD_LAZY, LOAD_NOW = RTLD_NOW, LOAD_NO_LOAD = RTLD_NOLOAD };

public:
    DllHelper() = default;
    ~DllHelper() {
        close();
    }

public:
    int open(const char *filename, LOAD_TYPE type);

    char *errMsg() const;

    void *GetSymbol(const char *symbolName);

    void close();

private:
    void *handle_{nullptr};
};

} // namespace muduo::base
