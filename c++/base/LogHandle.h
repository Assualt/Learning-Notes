#pragma once
#include "Format.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
// #include "noncopyable.h"
#define MAX_FILENAME_BUF 256

namespace muduo {
namespace base {

class LogHandle {
public:
    virtual void writeData(const char *pData, size_t nsize) {
    }
    virtual ~LogHandle() {
    }
};

class StdOutLogHandle : public LogHandle {
public:
    virtual void writeData(const char *pData, size_t nsize) {
        std::cout.write(pData, nsize) << std::flush;
    }
};

class RollingFileLogHandle : public LogHandle {
public:
    RollingFileLogHandle(const char *filePath, const char *filePrefix, const char *timepostfix = "%Y-%m-%d", int RotateFileSeconds = 86400)
        : m_strFilePathDir(filePath)
        , m_strFilePrefix(filePrefix)
        , m_strTimePostfix(timepostfix)
        , m_nRotateFileSeconds(RotateFileSeconds) {
    }

    virtual void writeData(const char *data, size_t nLength) {
        changeAccessFile();
        fout.write(data, nLength);
        fout.flush();
    }

    ~RollingFileLogHandle() {
        fout.close();
    }

protected:
    std::string CurrentFileName() {
        time_t      tNow(time(nullptr));
        char        FileNamebuf[ MAX_FILENAME_BUF ];
        std::string FileFormat = m_strFilePrefix + "-" + m_strTimePostfix;
        int         writeSize  = snprintf(FileNamebuf, 200, "%s-", m_strFilePrefix.c_str());
        strftime(FileNamebuf + writeSize, MAX_FILENAME_BUF - writeSize, m_strTimePostfix.c_str(), localtime(&tNow));
        return FileNamebuf;
    }

    bool changeAccessFile() {
        std::string currentFile = CurrentFileName();
        if (currentFile == m_strCurrentFile)
            return false;
        // close current file
        if (fout.is_open())
            fout.close();
        std::string FullFileName = m_strFilePathDir + "/" + currentFile;
        fout.open(FullFileName, std::ios::app);
        m_strCurrentFile = currentFile;
        return true;
    }

private:
    std::string m_strCurrentFile;

    std::string m_strFilePathDir;
    std::string m_strFilePrefix;
    std::string m_strTimePostfix;

    int m_nRotateFileSeconds;

    std::ofstream fout;
};

class RollingFile2LogHandle : public LogHandle {
public:
    RollingFile2LogHandle(const char *filepath, const char *dirPrefix, const char *filePrefix)
        : m_strFileRootPath(filepath)
        , m_strFileDirPrefix(dirPrefix)
        , m_strFilePrefix(filePrefix) {
    }

    std::string CurrentFile(std::string &dirpath) {
        time_t     t(time(nullptr));
        struct tm *ttime = localtime(&t);

        std::stringstream ss, ss1;
        ss << m_strFileDirPrefix << std::setw(4) << std::setfill('0') << ttime->tm_year + 1900 << "_" << std::setw(2) << std::setfill('0') << ttime->tm_mon << "_" << std::setw(2) << std::setfill('0')
           << ttime->tm_mday;

        ss1 << m_strFilePrefix << std::setw(2) << std::setfill('0') << ttime->tm_hour << "_00_00.log";

        dirpath          = FmtString("%/%").arg(m_strFileRootPath).arg(ss.str()).str();
        std::string file = FmtString("%/%").arg(dirpath).arg(ss1.str()).str();
        return file;
    }

    virtual void writeData(const char *data, size_t nLength) {
        std::string strDirPath;
        std::string currentFile = CurrentFile(strDirPath);
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
    std::string   m_strFileRootPath;
    std::string   m_strFileDirPrefix;
    std::string   m_strFilePrefix;
    std::string   m_strLastFile;
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
        if (fout.is_open())
            fout.close();
    }

private:
    size_t        m_nRollingFileSize;
    size_t        m_nCurrentFileSize;
    size_t        m_nIndex;
    std::string   m_strFilePrefix;
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
