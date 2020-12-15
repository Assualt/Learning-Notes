#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <time.h>
#include "httpconfig.h"
namespace http {
class HttpResponse {
public:
    typedef std::vector<std::pair<std::string, std::string>> ResourceMap;
    template <class T>
    void setHeader(const std::string &key, const T &val) {
        m_vResponseHeader.push_back(std::pair<std::string, std::string>(key, utils::toString(val)));
    }
    void setStatusMessage(int statusCode, const std::string &HttpVersion, const std::string &message) {
        m_nStatusCode = statusCode;
        m_strHttpVersion = HttpVersion;
        m_strMessage = message;
    }
    std::string toResponseHeader() {
        stringstream ss;
        ss << m_strHttpVersion << " " << m_nStatusCode << " " << m_strMessage << CTRL;
        for (auto &item : m_vResponseHeader)
            ss << item.first << ": " << item.second << CTRL;
        if (!m_strBodyString.empty())
            ss << CTRL << m_strBodyString << CTRL;
        ss << CTRL;
        return ss.str();
    }

public:
    std::string getBodyString() const {
        return m_strBodyString;
    }
    void setBodyString(const std::string &strBodyString) {
        m_strBodyString = strBodyString;
    }
    int getStatusCode() const {
        return m_nStatusCode;
    }
    void setStatusCode(int nStatusCode) {
        m_nStatusCode = nStatusCode;
    }
    std::string getBasicDate() const {
        return m_strBasicDate;
    }
    void setBasicDate(int strBasicDate) {
        m_strBasicDate = strBasicDate;
    }

    void WriteBytes(const char *buf, size_t n) {
        mybuf.sputn(buf, n);
    }

    MyStringBuffer &getBuffer() {
        return mybuf;
    }

    int loadBinaryFile(const std::string &strFilePath) {
        if (access(strFilePath.c_str(), F_OK) == -1)
            return -1;
        char ch;
        int nSize = 0;
        FILE *fp = nullptr;
        if ((fp = fopen(strFilePath.c_str(), "rb")) == nullptr) {
            logger.info("open %s binary file failed.", strFilePath);
            return 0;
        }
        logger.info("begin to load binaryFile %s", strFilePath);
        while (1) {
            ch = fgetc(fp);
            if (feof(fp))
                break;
            mybuf.sputc(ch);
            nSize++;
        }
        fclose(fp);
        return nSize;
    }

    int loadFileString(const std::string &strFilePath) {
        m_strBodyString = utils::loadFileString(strFilePath);
        return m_strBodyString.size();
    }

    size_t WriteBytes(int fd) {
        if (fd == -1)
            return 0;
        std::string strHeader = toResponseHeader();
        // write body
        size_t nHeaderSize = strHeader.size();
        int nBufferSize = mybuf.size();
        char *temp = new char[nHeaderSize + nBufferSize];
        memset(temp, 0, nBufferSize);
        strncpy(temp, strHeader.c_str(), strHeader.size());
        mybuf.seekReadPos(0);
        mybuf.sgetn(temp + strHeader.size(), nBufferSize);
        size_t nWrite = ::send(fd, temp, nHeaderSize + nBufferSize, 0);
        delete[] temp;
        return nWrite;
    }

private:
    // getter and setter
    std::string m_strBodyString;
    int m_nStatusCode;
    std::string m_strBasicDate;
    ResourceMap m_vResponseHeader;
    std::string m_strHttpVersion;
    std::string m_strMessage;

    MyStringBuffer mybuf;
};

}  // namespace http