//
// Created by xhou on 2022/10/30.
//
#include "Format.h"
#include "LogHandle.h"
#include "System.h"
#include "Timestamp.h"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#define BASIC_TIME_POSTFIX "%Y-%m-%d"

using std::string;
using std::stringstream;

using namespace muduo::base;

void StdOutLogHandle::writeData(const char *pData, size_t nSize) {
    std::cout.write(pData, static_cast<long>(nSize)).flush();
}
RollingFileLogHandle::RollingFileLogHandle(const char *filePath, const char *filePrefix, const char *timePostfix)
    : LogHandle()
    , m_strFilePathDir(filePath)
    , m_strFilePrefix(filePrefix)
    , m_strTimePostfix(timePostfix) {}

void RollingFileLogHandle::writeData(const char *data, size_t nLength) {
    changeAccessFile();
    output.write(data, static_cast<long>(nLength)).flush();
}

RollingFileLogHandle::~RollingFileLogHandle() { output.close(); }

string RollingFileLogHandle::getNextFileName() {
    Timestamp tNow             = Timestamp::fromUnixTime(time(nullptr));
    auto      formatTimeString = tNow.toFmtString(m_strTimePostfix.c_str());
    return FmtString("%.%").arg(m_strFilePrefix).arg(formatTimeString).str();
}

bool RollingFileLogHandle::changeAccessFile() {
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

RollingFile2LogHandle::RollingFile2LogHandle(const char *filepath, const char *dirPrefix, const char *filePrefix)
    : m_strFileRootPath(filepath)
    , m_strFileDirPrefix(dirPrefix)
    , m_strFilePrefix(filePrefix) {}

void RollingFile2LogHandle::writeData(const char *data, size_t nLength) {
    auto [ strDirPath, currentFile ] = getCurrentFile();
    if (!System::Access(strDirPath, F_OK)) {
        (void)System::Mkdir(strDirPath, DEFFILEMODE);
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

RollingFile2LogHandle::~RollingFile2LogHandle() { fout.close(); }

std::pair<std::string, std::string> RollingFile2LogHandle::getCurrentFile() {
    Timestamp now      = Timestamp::now();
    auto      dirPath  = m_strFileDirPrefix + FmtString("%/%").arg(now.toFmtString("%Y_%m_%d")).str();
    auto      filePath = FmtString("%/%_00_00.log").arg(dirPath).arg(now.toFmtString("%H")).str();
    return {dirPath, filePath};
}

RollingFileSizeLogHandle::RollingFileSizeLogHandle(const char *filepath, const char *filePrefix, size_t RollingFileSize)
    : m_nRollingFileSize(RollingFileSize)
    , m_ngetCurrentFileSize(0)
    , m_nIndex(0) {
    m_strFilePrefix = FmtString("%/%").arg(filepath).arg(filePrefix).str();
}

void RollingFileSizeLogHandle::writeData(const char *data, size_t nLength) {
    int leftBytes = 0;
    if (!changeAccessFile(data, nLength, leftBytes)) {
        fout.write(data, static_cast<long>(nLength)).flush();
    } else {
        fout.write(data + nLength - leftBytes, leftBytes).flush();
    }
}

bool RollingFileSizeLogHandle::changeAccessFile(const char *data, size_t curSize, int &leftBytes) {
    if (!fout.is_open()) {
        fout.open(FmtString("%.%").arg(m_strFilePrefix).arg(m_nIndex).str());
    }
    if (m_ngetCurrentFileSize + curSize <= m_nRollingFileSize) {
        m_ngetCurrentFileSize += curSize;
        return false;
    }
    auto writeBytes = m_nRollingFileSize - m_ngetCurrentFileSize;
    leftBytes       = curSize - writeBytes;
    fout.write(data, static_cast<long>(writeBytes)).flush();
    m_ngetCurrentFileSize = leftBytes;
    fout.close();
    m_nIndex++;
    fout.open(FmtString("%.%").arg(m_strFilePrefix).arg(m_nIndex).str());
    return true;
}

RollingFileSizeLogHandle::~RollingFileSizeLogHandle() {
    if (fout.is_open()) {
        fout.close();
    }
}
