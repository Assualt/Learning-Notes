#include "Dll.h"
#include <dlfcn.h>
#include <unistd.h>
using namespace muduo::base;

int DllHelper::open(const char *filename, LOAD_TYPE) {
    if (access(filename, F_OK) != 0) {
        return -1;
    }

    handle_ = dlopen(filename, RTLD_LAZY);
    if (handle_ == nullptr) {
        return -2;
    }

    return 0;
}

char *DllHelper::errMsg() const {
    return dlerror();
}

void *DllHelper::GetSymbol(const char *symbolName) {
    if (handle_ == nullptr) {
        return nullptr;
    }

    return dlsym(handle_, symbolName);
}

void DllHelper::close() {
    if (handle_ == nullptr) {
        return;
    }

    dlclose(handle_);
}