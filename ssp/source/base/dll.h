//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_DLL_H
#define SSP_TOOLS_DLL_H

#include <dlfcn.h>


namespace ssp::base {
enum LOAD_TYPE {
    LOAD_LAZY    = RTLD_LAZY,
    LOAD_NOW     = RTLD_NOW,
    LOAD_NO_LOAD = RTLD_NOLOAD,
};

class Dll {
public:
    Dll() = default;

    ~Dll() { Close(); }

public:
    int Open(const char *filename, LOAD_TYPE type = LOAD_TYPE::LOAD_LAZY);

    static char *ErrMsg() ;

    template<class Func>
    Func GetSymbol(const char *symbolName)
    {
        auto symbol = dlsym(handle_, symbolName);
        if (symbol == nullptr) {
            return nullptr;
        }

        return reinterpret_cast<Func>(symbol);
    }

    void Close();

private:
    void *handle_{nullptr};

};
}

#endif //SSP_TOOLS_DLL_H
