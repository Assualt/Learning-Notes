//
// Created by 侯鑫 on 2024/1/1.
//

#include "dll.h"

using namespace ssp::base;

int Dll::Open(const char *filename, LOAD_TYPE type)
{
    handle_ = dlopen(filename, type);
    if (handle_ == nullptr) {
        return -2;
    }

    return 0;
}

char *Dll::ErrMsg()
{
    return dlerror();
}

void Dll::Close()
{
    if (handle_ == nullptr) {
        return;
    }

    dlclose(handle_);
}