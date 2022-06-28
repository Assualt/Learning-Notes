#pragma once

namespace muduo {
namespace base {

class DllHelper {
public:
    enum LOAD_TYPE { LOAD_LAZY };

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
} // namespace base
} // namespace muduo
