#pragma once
#include "Exception.h"
#include "Format.h"
#include "Timestamp.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_FILENAME_BUF 256

#define SECONDS_DAY(n) (24 * n * 3600)
#define SECONDS_ONE_DAY SECONDS_DAY(1)
#define BASIC_TIME_POSTFIX "%Y-%m-%d"

using std::string;
using std::stringstream;

namespace muduo {
namespace base {

class LogHandle {
public:
    virtual void writeData(const char *pData, size_t nsize) {}
    virtual ~LogHandle()                                    = default;
};

class StdOutLogHandle : public LogHandle {
public:
    virtual void writeData(const char *pData, size_t nsize) {
        std::cout.write(pData, nsize) << std::flush;
    }
};

class RollingFileLogHandle : public LogHandle {
public:
    RollingFileLogHandle(const char *filePath, const char *filePrefix, const char *timePostfix = BASIC_TIME_POSTFIX)
        : LogHandle()
        , m_strFilePathDir(filePath)
        , m_strFilePrefix(filePrefix)
        , m_strTimePostfix(timePostfix) {
    }

    virtual void writeData(const char *data, size_t nLength) {
        changeAccessFile();
        foutputStream.write(data, nLength).flush();
    }

    ~RollingFileLogHandle() {
        foutputStream.close();
    }

protected:
    string CurrentFileName() {
        Timestamp tNow             = Timestamp::fromUnixTime(time(nullptr));
        auto      formatTimeString = tNow.toFormattedString(m_strTimePostfix.c_str());
        return FmtString("%_%").arg(m_strFilePrefix).arg(formatTimeString).str();
    }

    bool changeAccessFile() {
        string currentFile = CurrentFileName();
        if (currentFile == m_strCurrentFile)
            return false;
        if (foutputStream.is_open())
            foutputStream.close();
        string FullFileName = FmtString("%/%").arg(m_strFilePathDir).arg(currentFile).str();
        foutputStream.open(FullFileName, std::ios::app | std::ios::out);
        m_strCurrentFile = currentFile;
        return true;
    }

private:
    string        m_strCurrentFile; // current Log File Name
    string        m_strFilePathDir; // current Log File Path
    string        m_strFilePrefix;  // filePrefix
    string        m_strTimePostfix; //
    std::ofstream foutputStream;    // output stream
};

class RollingFile2LogHandle : public LogHandle {
public:
    RollingFile2LogHandle(const char *filepath, const char *dirPrefix, const char *filePrefix)
        : m_strFileRootPath(filepath)
        , m_strFileDirPrefix(dirPrefix)
        , m_strFilePrefix(filePrefix) {
    }

    string CurrentFile(string &dirpath) {
        time_t     t(time(nullptr));
        struct tm *ttime = localtime(&t);

        stringstream ss, ss1;
        ss << m_strFileDirPrefix << std::setw(4) << std::setfill('0') << ttime->tm_year + 1900 << "_" << std::setw(2) << std::setfill('0') << ttime->tm_mon << "_" << std::setw(2) << std::setfill('0')
           << ttime->tm_mday;

        ss1 << m_strFilePrefix << std::setw(2) << std::setfill('0') << ttime->tm_hour << "_00_00.log";

        dirpath     = FmtString("%/%").arg(m_strFileRootPath).arg(ss.str()).str();
        string file = FmtString("%/%").arg(dirpath).arg(ss1.str()).str();
        return file;
    }

    virtual void writeData(const char *data, size_t nLength) {
        string strDirPath;
        string currentFile = CurrentFile(strDirPath);
        if (access(strDirPath.c_str(), F_OK)) {
            mkdir(strDirPath.c_str(), 0666);
        }
        if (currentFile != m_strLastFile) {
            fout.close();
            m_strLastFile = currentFile;
        }
        if (!fout.is_open()) {
            fout.open(m_strLastFile);
        }
        fout.write(data, nLength).flush();
    }

    ~RollingFile2LogHandle() {
        fout.close();
    }

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
    virtual void writeData(const char *data, size_t nLength) {
        int nleftBytes = 0;
        if (!changeAccessFile(data, nLength, nleftBytes)) {
            fout.write(data, nLength).flush();
        } else {
            fout.write(data + nLength - nleftBytes, nleftBytes).flush();
        }
    }

    bool changeAccessFile(const char *data, size_t nCurrentSize, int &nleftBytes) {
        if (!fout.is_open()) {
            fout.open(FmtString("%.%").arg(m_strFilePrefix).arg(m_nIndex).str());
        }
        if (m_nCurrentFileSize + nCurrentSize <= m_nRollingFileSize) {
            m_nCurrentFileSize += nCurrentSize;
            return false;
        }
        int writeBytes = m_nRollingFileSize - m_nCurrentFileSize;
        nleftBytes     = nCurrentSize - writeBytes;
        fout.write(data, writeBytes).flush();
        m_nCurrentFileSize = nleftBytes;
        fout.close();
        m_nIndex++;
        fout.open(FmtString("%.%").arg(m_strFilePrefix).arg(m_nIndex).str());
        return true;
    }

    ~RollingFileSizeLogHandle() {
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

class AsyncHttpLogHandle : public StdOutLogHandle {
public:
    AsyncHttpLogHandle(const char *requestUrl, const char *requestType = "POST") {
    }

private:
};

} // namespace base
} // namespace muduo
