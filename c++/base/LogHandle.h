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
    void writeData(const char *pData, size_t nSize) override {
        std::cout.write(pData, static_cast<long>(nSize)).flush();
    }
};

class RollingFileLogHandle : public LogHandle {
public:
    RollingFileLogHandle(const char *filePath, const char *filePrefix, const char *timePostfix = BASIC_TIME_POSTFIX)
        : LogHandle()
        , m_strFilePathDir(filePath)
        , m_strFilePrefix(filePrefix)
        , m_strTimePostfix(timePostfix) {}

    void writeData(const char *data, size_t nLength) override {
        changeAccessFile();
        output.write(data, static_cast<long>(nLength)).flush();
    }

    ~RollingFileLogHandle() override { output.close(); }

protected:
    string getNextFileName() {
        Timestamp tNow             = Timestamp::fromUnixTime(time(nullptr));
        auto      formatTimeString = tNow.toFmtString(m_strTimePostfix.c_str());
        return FmtString("%.%").arg(m_strFilePrefix).arg(formatTimeString).str();
    }

    bool changeAccessFile() {
        string currentFile = getNextFileName();
        if (currentFile == m_strCurrentFile) {
            return false;
        }
        if (output.is_open()) {
            output.close();
        }
        string FullFileName = FmtString("%/%").arg(m_strFilePathDir).arg(currentFile).str();
        output.open(FullFileName, std::ios::app | std::ios::out);
        m_strCurrentFile = currentFile;
        return true;
    }

private:
    string        m_strCurrentFile; // current Log File Name
    string        m_strFilePathDir; // current Log File Path
    string        m_strFilePrefix;  // filePrefix
    string        m_strTimePostfix; //
    std::ofstream output;           // output stream
};

class RollingFile2LogHandle : public LogHandle {
public:
    RollingFile2LogHandle(const char *filepath, const char *dirPrefix, const char *filePrefix)
        : m_strFileRootPath(filepath)
        , m_strFileDirPrefix(dirPrefix)
        , m_strFilePrefix(filePrefix) {}

    string CurrentFile(string &dirPath) {
        time_t     t(time(nullptr));
        struct tm *lTime = localtime(&t);

        stringstream ss, ss1;
        ss << m_strFileDirPrefix << std::setw(4) << std::setfill('0') << lTime->tm_year + 1900 << "_" << std::setw(2)
           << std::setfill('0') << lTime->tm_mon << "_" << std::setw(2) << std::setfill('0') << lTime->tm_mday;

        ss1 << m_strFilePrefix << std::setw(2) << std::setfill('0') << lTime->tm_hour << "_00_00.log";

        dirPath     = FmtString("%/%").arg(m_strFileRootPath).arg(ss.str()).str();
        string file = FmtString("%/%").arg(dirPath).arg(ss1.str()).str();
        return file;
    }

    void writeData(const char *data, size_t nLength) override {
        string strDirPath;
        string currentFile = CurrentFile(strDirPath);
        if (access(strDirPath.c_str(), F_OK)) {
            System::mkdir(strDirPath, 0666);
        }
        if (currentFile != m_strLastFile) {
            fout.close();
            m_strLastFile = currentFile;
        }
        if (!fout.is_open()) {
            fout.open(m_strLastFile);
        }
        fout.write(data, static_cast<long>(nLength)).flush();
    }

    ~RollingFile2LogHandle() override { fout.close(); }

private:
    string        m_strFileRootPath;
    string        m_strFileDirPrefix;
    string        m_strFilePrefix;
    string        m_strLastFile;
    std::ofstream fout;
};

class RollingFileSizeLogHandle : public LogHandle {
public:
    RollingFileSizeLogHandle(const char *filepath, const char *filePrefix, size_t RollingFileSize)
        : m_nRollingFileSize(RollingFileSize)
        , m_nCurrentFileSize(0)
        , m_nIndex(0) {
        m_strFilePrefix = FmtString("%/%").arg(filepath).arg(filePrefix).str();
    }

    void writeData(const char *data, size_t nLength) override {
        int leftBytes = 0;
        if (!changeAccessFile(data, nLength, leftBytes)) {
            fout.write(data, static_cast<long>(nLength)).flush();
        } else {
            fout.write(data + nLength - leftBytes, leftBytes).flush();
        }
    }

    bool changeAccessFile(const char *data, size_t curSize, int &leftBytes) {
        if (!fout.is_open()) {
            fout.open(FmtString("%.%").arg(m_strFilePrefix).arg(m_nIndex).str());
        }
        if (m_nCurrentFileSize + curSize <= m_nRollingFileSize) {
            m_nCurrentFileSize += curSize;
            return false;
        }
        auto writeBytes = m_nRollingFileSize - m_nCurrentFileSize;
        leftBytes      = curSize - writeBytes;
        fout.write(data, static_cast<long>(writeBytes)).flush();
        m_nCurrentFileSize = leftBytes;
        fout.close();
        m_nIndex++;
        fout.open(FmtString("%.%").arg(m_strFilePrefix).arg(m_nIndex).str());
        return true;
    }

    ~RollingFileSizeLogHandle() override {
        if (fout.is_open()) {
            fout.close();
        }
    }

private:
    size_t        m_nRollingFileSize;
    size_t        m_nCurrentFileSize;
    size_t        m_nIndex;
    string        m_strFilePrefix;
    std::ofstream fout;
};

class [[maybe_unused]] AsyncHttpLogHandle : public StdOutLogHandle {};

} // namespace muduo::base
