#pragma once
#include "Exception.h"
#include "Format.h"
#include "System.h"
#include "Timestamp.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#define BASIC_TIME_POSTFIX "%Y-%m-%d"

using std::string;
using std::stringstream;

namespace muduo::base {

class LogHandle {
public:
    virtual void writeData(const char *pData, size_t nSize) {}
    virtual ~LogHandle() = default;
};

class StdOutLogHandle : public LogHandle {
public:
    void writeData(const char *pData, size_t nSize) override;
};

class RollingFileLogHandle : public LogHandle {
public:
    RollingFileLogHandle(const char *filePath, const char *filePrefix, const char *timePostfix = BASIC_TIME_POSTFIX);

    void writeData(const char *data, size_t nLength) override;

    ~RollingFileLogHandle() override;

private:
    string getNextFileName();

    bool changeAccessFile();

private:
    string        m_strCurrentFile; // current Log File Name
    string        m_strFilePathDir; // current Log File Path
    string        m_strFilePrefix;  // filePrefix
    string        m_strTimePostfix; //
    std::ofstream output;           // output stream
};

class RollingFile2LogHandle : public LogHandle {
public:
    RollingFile2LogHandle(const char *filepath, const char *dirPrefix, const char *filePrefix);
    void writeData(const char *data, size_t nLength) override;
    ~RollingFile2LogHandle() override;

private:
    std::pair<std::string, std::string> getCurrentFile();

private:
    string        m_strFileRootPath;
    string        m_strFileDirPrefix;
    string        m_strFilePrefix;
    string        m_strLastFile;
    std::ofstream fout;
};

class RollingFileSizeLogHandle : public LogHandle {
public:
    RollingFileSizeLogHandle(const char *filepath, const char *filePrefix, size_t RollingFileSize);

    void writeData(const char *data, size_t nLength) override;

    bool changeAccessFile(const char *data, size_t curSize, int &leftBytes);

    ~RollingFileSizeLogHandle() override;

private:
    size_t        m_nRollingFileSize;
    size_t        m_ngetCurrentFileSize;
    size_t        m_nIndex;
    string        m_strFilePrefix;
    std::ofstream fout;
};

class [[maybe_unused]] AsyncHttpLogHandle : public StdOutLogHandle{};

} // namespace muduo::base
