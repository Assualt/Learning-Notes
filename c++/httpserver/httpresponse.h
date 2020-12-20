#pragma once
#include "httpconfig.h"
#include "httputils.h"
#include <iostream>
#include <sstream>
#include <time.h>
#include <vector>
#define CHUNK_SIGNLE_SIZE 1024
namespace http {
class HttpResponse {
public:
    typedef std::vector<std::pair<std::string, std::string>> ResourceMap;
    template <class T> void                                  setHeader(const std::string &key, const T &val) {
        m_vResponseHeader.push_back(std::pair<std::string, std::string>(key, utils::toString(val)));
    }
    void        setStatusMessage(int statusCode, const std::string &HttpVersion, const std::string &message, const std::string &strAcceptEncoding = "");
    std::string toResponseHeader();

public:
    std::string getBodyString() const;
    void        setBodyString(const std::string &strBodyString);
    int         getStatusCode() const;
    void        setStatusCode(int nStatusCode);
    std::string getBasicDate() const;
    void        setBasicDate(int strBasicDate);

    void            WriteBodyBytes(const char *buf, size_t n);
    MyStringBuffer &getBuffer();

    int    loadBinaryFile(const std::string &strFilePath);
    int    loadFileString(const std::string &strFilePath);
    size_t WriteBytes(int fd);

protected:
    void    EncodeBodyString(MyStringBuffer &out, int CompressType);
    ssize_t ChunkString(MyStringBuffer &buffer, int nChunkSize, MyStringBuffer &ss);

private:
    // getter and setter
    std::string m_strBodyString;
    int         m_nStatusCode;
    std::string m_strBasicDate;
    ResourceMap m_vResponseHeader;
    std::string m_strHttpVersion;
    std::string m_strMessage;

    MyStringBuffer        mybuf;
    std::set<std::string> m_AcceptEncodingSet;
};

} // namespace http