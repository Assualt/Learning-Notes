#include "httpresponse.h"
#include "hashutils.hpp"
#include "logging.h"
#include <string.h>
#include <sys/socket.h>
namespace http {

void HttpResponse::setStatusMessage(int statusCode, const std::string &HttpVersion, const std::string &message, const std::string &strAcceptEncoding) {
    m_nStatusCode      = statusCode;
    m_strHttpVersion   = HttpVersion;
    m_strMessage       = message;
    auto EncodingItems = utils::split(strAcceptEncoding, ',');
    std::for_each(EncodingItems.begin(), EncodingItems.end(), [ this ](std::string &item) { this->m_AcceptEncodingSet.insert(item); });
}
std::string HttpResponse::toResponseHeader() {
    stringstream ss;
    ss << m_strHttpVersion << " " << m_nStatusCode << " " << m_strMessage << CTRL;
    for (auto &item : m_vResponseHeader)
        ss << item.first << ": " << item.second << CTRL;
    ss << "Date: " << utils::toResponseBasicDateString() << CTRL;
    ss << SERVER << ": " << SERVERVal << CTRL;
    ss << CTRL;
    return ss.str();
}

std::string HttpResponse::getBodyString() const {
    return m_strBodyString;
}
void HttpResponse::setBodyString(const std::string &strBodyString) {
    m_strBodyString = strBodyString;
}
int HttpResponse::getStatusCode() const {
    return m_nStatusCode;
}
void HttpResponse::setStatusCode(int nStatusCode) {
    m_nStatusCode = nStatusCode;
}
std::string HttpResponse::getBasicDate() const {
    return m_strBasicDate;
}
void HttpResponse::setBasicDate(int strBasicDate) {
    m_strBasicDate = strBasicDate;
}

void HttpResponse::WriteBodyBytes(const char *buf, size_t n) {
    mybuf.sputn(buf, n);
}

MyStringBuffer &HttpResponse::getBuffer() {
    return mybuf;
}

int HttpResponse::loadBinaryFile(const std::string &strFilePath) {
    if (access(strFilePath.c_str(), F_OK) == -1)
        return -1;
    char  ch;
    int   nSize = 0;
    FILE *fp    = nullptr;
    if ((fp = fopen(strFilePath.c_str(), "rb")) == nullptr) {
        logger.info("open %s binary file failed.", strFilePath);
        return 0;
    }
    logger.debug("begin to load binaryFile %s", strFilePath);
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

int HttpResponse::loadFileString(const std::string &strFilePath) {
    m_strBodyString = utils::loadFileString(strFilePath);
    return m_strBodyString.size();
}

void HttpResponse::EncodeBodyString(MyStringBuffer &out, int CompressType) {
    // 0 no-compress
    // 1 gzip
    // 2 deflate
    // 3 br
    int   nBodySize   = m_strBodyString.size() + mybuf.size();
    char *bodyTempBuf = new char[ nBodySize ];
    memset(bodyTempBuf, 0, nBodySize);
    strncpy(bodyTempBuf, m_strBodyString.c_str(), m_strBodyString.size());
    mybuf.sgetn(bodyTempBuf + m_strBodyString.size(), mybuf.size());
    uLongf CompressedSize;
    if (CompressType == 1) { // gzip
        CompressedSize = HashUtils::GzipCompress(bodyTempBuf, nBodySize, out);
    } else if (CompressType == 2) { // deflate
        CompressedSize = HashUtils::GzipCompress(bodyTempBuf, nBodySize, out);
    } else {
        logger.debug("unsupported compress type");
    }
    logger.debug("compress %d bytes. encoded %d..", nBodySize, CompressedSize);
    delete[] bodyTempBuf;
}
ssize_t HttpResponse::ChunkString(MyStringBuffer &buffer, int nChunkSize, MyStringBuffer &ss) {
    size_t      nSendBytes = 0;
    std::string strHeader  = toResponseHeader();
    buffer.seekReadPos(0);
    char *temp = new char[ nChunkSize ];
    ss.sputn(strHeader.c_str(), strHeader.size());
    nSendBytes += strHeader.size();
    while (1) {
        memset(temp, 0, nChunkSize);
        size_t nRead = buffer.sgetn(temp, nChunkSize);
        if (nRead <= 0)
            break;
        std::string hexString = utils::toHexString(nRead);
        ss.sputn(hexString.c_str(), hexString.size());
        ss.sputn("\r\n", 2);
        ss.sputn(temp, nRead);
        ss.sputn("\r\n", 2);
        nSendBytes += 4 + hexString.size() + nRead;
    }
    ss.sputn("0\r\n\r\n", 7);
    delete[] temp;
    return nSendBytes + 7;
}

size_t HttpResponse::WriteBytes(int fd) {
    if (fd == -1)
        return 0;
    int CompressType = 0;
    // 0 no-compress
    // 1 gzip
    // 2 deflate
    // 3 br
    if (strcasecmp(m_strHttpVersion.c_str(), "HTTP/1.0") == 0) // use Content-Length
        CompressType = 0;
    else if (strcasecmp(m_strHttpVersion.c_str(), "HTTP/1.1") == 0) {
        if (m_AcceptEncodingSet.count("gzip"))
            CompressType = 1;
        else if (m_AcceptEncodingSet.count("deflate"))
            CompressType = 2;
        else if (m_AcceptEncodingSet.count("br"))
            CompressType = 3;
    }
    std::string strHeader   = toResponseHeader();
    size_t      nHeaderSize = strHeader.size();
    int         nBufferSize = mybuf.size();
    int         bodySize    = nBufferSize + m_strBodyString.size();
    int         nTotalSend  = nHeaderSize + nBufferSize + m_strBodyString.size();
    if (CompressType == 0) {
        std::stringstream ss;
        ss << strHeader << m_strBodyString;
        char *temp = new char[ nBufferSize ];
        memset(temp, 0, nBufferSize);
        size_t nRead = mybuf.sgetn(temp, nBufferSize);
        ss.write(temp, nRead);
        size_t nWrite = ::send(fd, ss.str().c_str(), ss.str().size(), 0);
        delete[] temp;
        return nWrite;
    } else if (CompressType == 1 || CompressType == 2) { // gzip
        MyStringBuffer EncodeBuffer, outBuffer;
        EncodeBodyString(EncodeBuffer, CompressType);
        std::stringstream ss;
        setHeader(TransferEncoding, "chunked");
        if (CompressType == 1)
            setHeader(ContentEncoding, "gzip");
        else if (CompressType == 2)
            setHeader(ContentEncoding, "deflate");
        ssize_t nSendBytes = ChunkString(EncodeBuffer, CHUNK_SIGNLE_SIZE, outBuffer);
        char *  temp       = new char[ nSendBytes ];
        memset(temp, 0, nSendBytes);
        outBuffer.seekReadPos(0);
        outBuffer.sgetn(temp, nSendBytes);
        size_t nWrite = send(fd, temp, nSendBytes, 0);
        // for (int i = 0; i < nSendBytes; i++) {
        //     if (temp[ i ] != '\n')
        //         printf("%02x ", temp[ i ] & 0xFF);
        //     else if (temp[ i - 1 ] == '\r' && temp[ i ] == '\n')
        //         printf("%02x\n", temp[ i ] & 0xFF);
        // }
        delete[] temp;
        return nWrite;
    } else {
        logger.warning("unsupported type.");
        return 0;
    }
}

} // namespace http