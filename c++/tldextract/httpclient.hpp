#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netdb.h> /* gethostbyname */
#include <netinet/in.h>
#include <sstream>
#include <streambuf>
#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */
#include <unistd.h>
#include <vector>

#ifdef USE_GURL
#include "gurl/url/gurl.h"
#else
#include "url/url.h"
#endif
#include "hashutils.hpp"
#include "logging.h"
using namespace std;

#ifdef USE_OPENSSL
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#endif

#define CTRL "\r\n"
#define ContentType "Content-Type"
#define ContentEncoding "Content-Encoding"
#define Accept "Accept"
#define AcceptEncoding "Accept-Encoding"
#define Authorization "Authorization"
#define AcceptLanguage "Accept-Language"
#define UserAgent "User-Agent"
#define CotentLength "Content-Length"
#define TransferEncoding "Transfer-Encoding"
#define AcceptRanges "Accept-Ranges"
#define Location "Location"
#define Cookie "Cookie"

#define HTTP1_1 "HTTP/1.1"
#define HTTP1_0 "HTTP/1.0"

#define TempFile ".httpclient.download"

namespace http
{
    typedef enum
    {
        EncodingLength,
        EncodingChunk,
        EncodingGzip,
        EncodingOther
    } Encoding;

    namespace utils
    {

        enum HttpVersion
        {
            HTTP_1_0,
            HTTP_1_1
        };

        static std::string _ltrim(const std::string &src, char ch = ' ')
        {
            std::string temp = src;
            std::string::iterator p = std::find_if(temp.begin(), temp.end(), [&ch](char c) { return ch != c; });
            temp.erase(temp.begin(), p);
            return temp;
        }

        static std::string _rtrim(const std::string &src, char ch = ' ')
        {
            string temp = src;
            string::reverse_iterator p = find_if(temp.rbegin(), temp.rend(), [&ch](char c) { return ch != c; });
            temp.erase(p.base(), temp.end());
            return temp;
        }

        static std::string trim(const std::string &src, char ch = ' ')
        {
            return _rtrim(_ltrim(src, ch), ch);
        }

        static size_t chunkSize(const std::string &strChunkSize)
        {
            std::string temp;
            for (size_t i = 0; i < strChunkSize.size(); i++)
            {
                if ((strChunkSize[i] >= '0' && strChunkSize[i] <= '9') || (strChunkSize[i] >= 'A' && strChunkSize[i] <= 'F') ||
                    (strChunkSize[i] >= 'a' && strChunkSize[i] <= 'f'))
                {
                    temp.push_back(strChunkSize[i]);
                }
                else
                {
                    break;
                }
            }
            if (temp.empty())
                return -1;
            return std::stoi(temp, nullptr, 16);
        }
    } // namespace utils

    struct HttpResource;
    struct HttpUrl
    {
    public:
        std::string scheme;
        std::string username;
        std::string password;
        std::string host;
        int port;
        std::string path;
        std::string query;
        std::string fragment;
        std::string fullurl;
        std::string netloc;

    public:
        HttpUrl(const std::string &url)
            : fullurl(url), scheme("http"), port(80)
        {
            parse();
        }
        HttpUrl() = default;

    private:
        void parse()
        {
            if (fullurl.find("://") == std::string::npos)
                fullurl = "http://" + fullurl;
#ifdef USE_GURL
            GURL url(fullurl);
            if (!url.is_valid() || !url.IsStandard())
                return;
            if (url.has_scheme())
                scheme = url.scheme();
            if (url.has_username())
                username = url.username();
            if (url.has_password())
                password = url.password();
            if (url.has_host())
                host = url.host();
            if (url.has_port())
                port = atoi(url.port().c_str());
            if (url.has_path())
                path = url.path();
            if (url.has_query())
                query = url.query();
            if (url.has_ref())
                fragment = url.ref();
#else
            Url url = Url::create(fullurl);
            host = url.getHost();
            if(host.empty())
                return;
            scheme = url.getScheme();
            username = url.getUsername();
            password = url.getPassword();
            port = url.getPort();
            path = url.getPath();
            query = url.getQuery();
            fragment = url.getFragment();
#endif
            netloc = host + ":" + to_string(port);
        }
    };

    struct HttpResource
    {
    public:
        typedef std::vector<std::pair<std::string, std::string>> ResourceMap;
        HttpResource()
            : m_nBodyBytes(0), m_nBodyDecodeBytes(0), m_bDecodeBodyStatus(false)
        {
        }

        void set(const std::string &key, const std::string &val)
        {
            ReqestHeader.push_back(std::pair<std::string, std::string>(key, val));
        }

        void setBody(const std::string &key, const std::string &val)
        {
            ResponseBody.push_back(std::pair<std::string, std::string>(key, val));
        }

        std::string getResponseText() const
        {
            return m_strResponseText;
        }

        void getResponseBytes(std::ostream &os)
        {
            char *WriteBuffer = new char[MAX_BUF_SIZE];
            m_ResponseBuffer.seekReadPos(0);
            while (1)
            {
                memset(WriteBuffer, 0, MAX_BUF_SIZE);
                std::streamsize nReadSize = m_ResponseBuffer.sgetn(WriteBuffer, MAX_BUF_SIZE);
                if (nReadSize == 0)
                    break;
                os.write(WriteBuffer, nReadSize);
            }
            m_ResponseBuffer.seekReadPos(0);
            delete[] WriteBuffer;
        }

        void setRequestType(const std::string &reqType)
        {
            m_strRequestType = reqType;
        }

        void setRequestPath(const std::string &reqPath)
        {
            m_strRequestPath = reqPath;
        }

        std::string getRequestPath() const
        {
            return m_strRequestPath;
        }

        void setHttpVersion(const std::string &httpversion)
        {
            m_strRequestHttpVersion = httpversion;
        }

        void setParams(const std::string &params)
        {
            m_strRequestParams = params;
        }

        size_t getResponseBytesSize() const
        {
            return m_nBodyBytes;
        }

        size_t getResponseDecodeBytesSize() const
        {
            return m_nBodyDecodeBytes;
        }

        std::string toStringHeader()
        {
            stringstream ss;
            ss << m_strRequestType << " " << m_strRequestPath << " " << m_strRequestHttpVersion << CTRL;
            for (auto &item : ReqestHeader)
            {
                ss << item.first << ": " << item.second << CTRL;
            }
            ss << CTRL;
            ss << m_strRequestParams << CTRL;
            return ss.str();
        }

        friend ostream &operator<<(ostream &os, HttpResource &obj)
        {
            if (!obj.ReqestHeader.empty())
                os << "> " << obj.m_strRequestType << " " << obj.m_strRequestPath << " " << obj.m_strRequestHttpVersion << CTRL;
            for (auto &item : obj.ReqestHeader)
            {
                os << "> " << item.first << ": " << item.second << CTRL;
            }
            for (auto &item : obj.ResponseBody)
            {
                os << "< " << item.first << ": " << item.second << CTRL;
            }
            if (!obj.ReqestHeader.empty() && !obj.m_strRequestParams.empty())
            {
                os << CTRL;
                os << obj.m_strRequestParams << CTRL;
            }
            if (!obj.m_strResponseText.empty())
            {
                os << "<\r\n";
                if (obj.getResponseItem(ContentType).find("text/") != std::string::npos)
                    os << obj.m_strResponseText << CTRL;
                else
                    os << "[Binary] " << obj.m_nBodyBytes << "Bytes" << CTRL;
            }
            // os << CTRL << "Recv Body Size:" << obj.m_nBodyBytes << " Decode Bytes:" << obj.m_nBodyDecodeBytes << std::endl;
            return os;
        }

        std::string getResponseItem(const std::string &key)
        {
            for (auto &item : ResponseBody)
            {
                if (strcasecmp(item.first.c_str(), key.c_str()) == 0)
                    return item.second;
            }
            return "";
        }

        std::string getCookie()
        {
            for (auto &item : ResponseBody)
            {
                std::string firstItem = item.first;
                std::transform(firstItem.begin(), firstItem.end(), firstItem.begin(), ::tolower);
                if (firstItem.find("cookie") != std::string::npos)
                    return item.second;
            }
            return "";
        }

        void WriteBodyBytes(const char *buf, size_t nBytes)
        {
            m_ResponseBuffer.sputn(buf, nBytes);
            m_nBodyBytes += nBytes;
            m_strResponseText.append(std::string(buf, nBytes));
            m_nBodyDecodeBytes += nBytes;
        }

        void WriteBodyByte(char ch)
        {
            m_ResponseBuffer.sputc(ch);
            m_nBodyBytes += 1;
            m_strResponseText.push_back(ch);
            m_nBodyDecodeBytes += 1;
        }

        void tryDecodeBody()
        {
            if (this->getResponseItem(ContentEncoding) == "gzip")
            {
                m_ResponseBuffer.seekReadPos(0);
                logger.info("Try Decode Bytes %d for gzip.", m_nBodyBytes);
                std::stringstream strstring;
                size_t nBytes = HashUtils::GzipDecompress(m_ResponseBuffer, strstring);
                if (nBytes)
                {
                    logger.info("Try Decode Bytes %d, and Decoded Bytes %d", m_nBodyBytes, nBytes);
                    m_strResponseText = strstring.str();
                    m_nBodyDecodeBytes = m_strResponseText.size();
                    m_bDecodeBodyStatus = true;
                }
                else
                {
                    logger.info("Decode Gzip Error.. bytes:%d", nBytes);
                }
            }
            else if (this->getResponseItem(ContentEncoding) == "deflate")
            {
                m_ResponseBuffer.seekReadPos(0);
                logger.info("Try Decode Bytes %d for deflate.", m_nBodyBytes);
                std::stringstream strstring;
                size_t nBytes = HashUtils::DeflateDecompress(m_ResponseBuffer, strstring);
                if (nBytes)
                {
                    logger.info("Try Decode Bytes %d, and Decoded Bytes %d", m_nBodyBytes, nBytes);
                    m_strResponseText = strstring.str();
                    m_nBodyDecodeBytes = m_strResponseText.size();
                    m_bDecodeBodyStatus = true;
                }
                else
                {
                    logger.info("Decode deflate Error.. bytes:%d", nBytes);
                }
            }
        }

        bool BodyIsDecoded() const
        {
            return m_bDecodeBodyStatus;
        }

        void clear()
        {
            ReqestHeader.clear();
            ResponseBody.clear();
            m_nBodyBytes = m_nBodyDecodeBytes = 0;
            m_ResponseBuffer.clear();
        }

        const ResourceMap &GetResponse() const{
            return ResponseBody;
        }

    private:
        
        ResourceMap ReqestHeader;
        ResourceMap ResponseBody;
        std::string m_strRequestType, m_strRequestPath, m_strRequestHttpVersion;
        std::string m_strRequestParams, m_strResponseText;
        ssize_t m_nBodyBytes, m_nBodyDecodeBytes;
        MyStringBuffer m_ResponseBuffer;
        bool m_bDecodeBodyStatus;
    };

    struct HttpResult
    {
    private:
        int m_nStatusCode;
        std::string m_strText;
        std::string m_strRelayError;

    public:
        HttpResult(int code, const std::string &text, const std::string &relayError)
            : m_nStatusCode(code), m_strText(text), m_strRelayError(relayError)
        {
        }

        int status_code() const
        {
            return m_nStatusCode;
        }

        std::string text() const
        {
            return m_strText;
        }

        std::string error() const
        {
            return m_strRelayError;
        }
    };

    enum HTTP_TYPE
    {
        TYPE_GET,
        TYPE_POST,
        TYPE_DELETE,
        TYPE_PUT
    };
#define MAX_SIZE 16384
    class SocketClient
    {
    public:
        SocketClient()
            : m_nConnectFd(-1), m_nChunkSize(0), m_nConnectTimeout(10), m_bUseSSL(false), m_bConnected(false)
        {
#ifdef USE_OPENSSL
            initSSL();
            m_pConnection = nullptr;
#endif
        }
        ~SocketClient()
        {
            disconnect();
            m_bConnected = false;
        }

    private:
#ifdef USE_OPENSSL
        void initSSL()
        {
            // Register the error strings for libcrypto & libssl
            SSL_load_error_strings();

            // Register the available ciphers and digests
            SSL_library_init();

            OpenSSL_add_all_algorithms();
        }
        bool SSLConnect()
        {

            m_pConnection = new SSL_Connection();
            m_pConnection->m_ptrContext = SSL_CTX_new(SSLv23_client_method());
            if (m_pConnection->m_ptrContext == nullptr)
            {
                ERR_print_errors_fp(stderr);
                return false;
            }
            // Create an SSL struct for the connection
            m_pConnection->m_ptrHandle = SSL_new(m_pConnection->m_ptrContext);
            if (m_pConnection->m_ptrHandle == nullptr)
            {
                ERR_print_errors_fp(stderr);
                return false;
            }

            // Connect the SSL struct to our connection
            if (!SSL_set_fd(m_pConnection->m_ptrHandle, m_nConnectFd))
            {
                ERR_print_errors_fp(stderr);
                return false;
            }
            // Initiate SSL handshake
            if (SSL_connect(m_pConnection->m_ptrHandle) != 1)
            {
                ERR_print_errors_fp(stderr);
                return false;
            }
            return true;
        }

        void SSLDisConnect()
        {
            if (m_pConnection)
            {
                if (m_pConnection->m_ptrHandle)
                {
                    SSL_shutdown(m_pConnection->m_ptrHandle);
                    SSL_free(m_pConnection->m_ptrHandle);
                }
                if (m_pConnection->m_ptrContext)
                    SSL_CTX_free(m_pConnection->m_ptrContext);

                delete m_pConnection;
            }
        }

        ssize_t SSLRecv(int fd, void *buf, ssize_t size)
        {
            return SSL_read(m_pConnection->m_ptrHandle, buf, size);
        }

        ssize_t SSLSend(const void *buf, ssize_t size)
        {
            if (m_pConnection != nullptr)
            {
                return SSL_write(m_pConnection->m_ptrHandle, buf, size);
            }
            return 0;
        }

#endif

        int RecvChunkData(const char *buffer, size_t &nPos, int size, HttpResource &Response, int &ChunkState, size_t &ChunkSize,
                          ssize_t &AllChunkBodySize, size_t &blockCount)
        {
            size_t nWriteBytes = 0;
            std::string strChunkSize;
            for (; nPos < size;)
            {
                if (ChunkState == 0)
                {
                    if ((nPos + 1) < size && buffer[nPos] == '\r' && buffer[nPos + 1] == '\n')
                    {
                        ChunkSize = utils::chunkSize(strChunkSize);
                        m_nChunkSize = ChunkSize;
                        ChunkState = 1;
                        // printf(" %02x %02x ChunkSize:%d %s\n",buffer[nPos], buffer[nPos+1], ChunkSize, strChunkSize.c_str());
                        nPos += 2;
                        if (ChunkSize == 0)
                        {
                            logger.debug("read chunkdata size is  0. Exit now..");
                            return 0;
                        }
                        else if (ChunkSize == -1)
                        {
                            logger.error("Recv Bytes Error. Exit now...");
                            return 0;
                        }
                    }
                    else
                    {
                        strChunkSize.push_back(buffer[nPos]);
                        // printf("+ \033[31m%02x\033[0m ", buffer[ nPos ]);
                        nPos += 1;
                    }
                }
                else if (ChunkState == 1)
                {
                    if ((nPos + 1) < size && buffer[nPos] == '\r' && buffer[nPos + 1] == '\n' && ChunkSize == 0)
                    {
                        ChunkState = 2;
                        nPos += 2;
                    }
                    else if (ChunkSize)
                    {
                        // printf("%02x ", buffer[nPos] & 0xFF);
                        Response.WriteBodyByte(buffer[nPos] & 0xFF);
                        AllChunkBodySize += 1;
                        nPos += 1;
                        nWriteBytes++;
                        ChunkSize--;
                    }
                    else
                    {
                    }
                }
                else if (ChunkState == 2)
                {
                    ChunkState = 0;
                    // printf("%d \n", m_nChunkSize);
                    logger.debug("Recv %dth block data, current block size:%d total recv blocks Bytes:%d", ++blockCount, m_nChunkSize, AllChunkBodySize);
                    strChunkSize.clear();
                }
            }
            char tempBuf[MAX_BUF_SIZE];
            size_t nRead = recvData(m_nConnectFd, tempBuf, MAX_BUF_SIZE, 0);
            nPos = 0;
            RecvChunkData(tempBuf, nPos, nRead, Response, ChunkState, ChunkSize, AllChunkBodySize, blockCount);
            return nWriteBytes;
        }

        Encoding ParseHeader(const char *buffer, ssize_t size, HttpResource &Response, ssize_t &HeaderSize, ssize_t &recvBodySize, ssize_t &LeftBodySize,
                             ssize_t &BodySizeInResponse)
        {
            std::string strKey, strVal, strLine;
            int LineCnt = 0;
            bool bFindBody = false;
            Encoding EncodingType = EncodingGzip; // 0: Content-Length 1: Chunked 2: other way
            int nCnt = 0;
            MyStringBuffer chunkedBuffer;
            int chunkState = 0;
            int chunkSize = 0;
            std::string strChunkSize;

            for (size_t i = 0; i < size;)
            {
                if (i + 1 < size && buffer[i] == '\r' && buffer[i + 1] == '\n' && !bFindBody)
                {
                    if (LineCnt == 0)
                    {
                        std::string HttpVersion, statusMessage;
                        int StatusCode;
                        ParseFirstLine(strLine, HttpVersion, StatusCode, statusMessage);
                        Response.setBody("code", to_string(StatusCode));
                        Response.setBody("message", statusMessage);
                        LineCnt++;
                    }
                    else if (!strLine.empty())
                    {
                        ParseHeaderLine(strLine, strKey, strVal);
                        strKey = utils::trim(strKey);
                        strVal = utils::trim(strVal);
                        // logger.debug("Find Response Body Header: %s->%s", strKey, strVal);
                        if (strKey == CotentLength)
                        {
                            BodySizeInResponse = atoi(strVal.c_str());
                            EncodingType = EncodingLength;
                        }
                        else if (strKey == TransferEncoding && strVal == "chunked")
                        {
                            EncodingType = EncodingChunk;
                        }
                        Response.setBody(strKey, strVal);
                    }
                    else if (strLine.empty())
                    {
                        bFindBody = true;
                    }
                    i += 2;
                    HeaderSize += strLine.size() + 2;
                    strLine.clear();
                }
                else if (!bFindBody)
                { // Header
                    strLine.push_back(buffer[i]);
                    i++;
                }
                else if (EncodingType != EncodingChunk && bFindBody)
                {
                    Response.WriteBodyByte(buffer[i] & 0xFF);
                    recvBodySize += 1;
                    i += 1;
                }
                else if (EncodingType == EncodingChunk && bFindBody)
                {
                    int ChunkState = 0;
                    size_t chunkSize = 0;
                    size_t blockCount = 0;
                    // Server Apache mode_deflate is not suitable for zlib.
                    RecvChunkData(buffer, i, size, Response, ChunkState, chunkSize, recvBodySize, blockCount);
                    break;
                }
            }
            if (EncodingType == EncodingLength && BodySizeInResponse != 0 && BodySizeInResponse != -1)
            {
                LeftBodySize = BodySizeInResponse - recvBodySize;
                logger.info("Header Size:%d recvBodySize:%d totalSize:%d BodySizeInResponse:%d LeftBodySize:%d EncodingType:%d", HeaderSize, recvBodySize,
                            size, BodySizeInResponse, LeftBodySize, EncodingType);
            }
            else if (EncodingType == EncodingChunk)
            {
                // printf("\n");
            }
            else if (BodySizeInResponse == -1 && Response.getResponseItem(ContentEncoding) == "gzip")
            {
                EncodingType = EncodingGzip;
            }
            return EncodingType;
        }
        void ParseHeaderLine(const std::string &line, std::string &key, std::string &val)
        {
            int nBlankCnt = 0;
            key.clear();
            val.clear();
            for (size_t i = 0; i < line.size(); i++)
            {
                if (line[i] == ':')
                    nBlankCnt = 1;
                else if (nBlankCnt == 0)
                {
                    key.push_back(line[i]);
                }
                else if (nBlankCnt == 1)
                {
                    val.push_back(line[i]);
                }
            }
        }

        void ParseFirstLine(const std::string &line, std::string &HttpVersion, int &StatusCode, std::string &StatusMessage)
        {
            int nBlankCnt = 0;
            StatusCode = 0;
            for (size_t i = 0; i < line.size(); i++)
            {
                if (line[i] == ' ')
                {
                    if (nBlankCnt != 2)
                        nBlankCnt++;
                    else
                        StatusMessage.push_back(line[i]);
                }
                else if (nBlankCnt == 0)
                {
                    HttpVersion.push_back(line[i]);
                }
                else if (nBlankCnt == 1)
                {
                    StatusCode = StatusCode * 10 + line[i] - '0';
                }
                else if (nBlankCnt == 2)
                {
                    StatusMessage.push_back(line[i]);
                }
            }
        }

    public:
        ssize_t read(HttpResource &Response)
        {
            auto tBegin = std::chrono::system_clock::now();
            char tempBuf[MAX_SIZE];
            ssize_t HeaderSize = 0, BodySize = 0;
            ssize_t recvBodySize = 0;
            ssize_t nRead = recvData(m_nConnectFd, tempBuf, MAX_SIZE, 0);
            if (nRead < 0)
            {
                return 0;
            }
            size_t nTotal = 0;
            // GET Content Length From first Buffer
            std::string temp;
            ssize_t LeftSize = -1;
            double speed;
            ssize_t BodySizeInResponse = -1;
            // ParseHeader 中 chunk模式会收集完所有数据后才返回最终结果
            // 其他模式直解析第一次获取的字节数,不做后续的解析
            Encoding EncodingType = ParseHeader(tempBuf, nRead, Response, HeaderSize, BodySize, LeftSize, BodySizeInResponse);
            recvBodySize += BodySize;
            if (EncodingType == EncodingLength)
            {
                nTotal = nRead;
            }
            else if (EncodingType == EncodingChunk)
                nTotal = nRead + BodySize;
            if (LeftSize <= 0 && EncodingType == EncodingLength)
            {
                logger.debug("Request Finished.");
            }
            else if (EncodingType == EncodingLength)
            {
                while (LeftSize > 0)
                {
                    memset(tempBuf, 0, MAX_SIZE);
                    nRead = recvData(m_nConnectFd, tempBuf, MAX_SIZE, 0);
                    if (nRead <= 0)
                        break;

                    Response.WriteBodyBytes(tempBuf, nRead);
                    recvBodySize += nRead;
                    nTotal += nRead;
                    LeftSize -= nRead;
                    BodySize += nRead;
                    speed = getCurrentSpeed(tBegin, recvBodySize, 'k');

                    double leftTime = LeftSize / 1000.0 / speed;
                    double Progress = recvBodySize * 100.0 / BodySizeInResponse;
                    logger.debug("Process:%.2f%%, Recv Size:%d bytes , LeftSize:%d bytes, recvBodySize:%d, speed:%.2f kb/s left time:%.2fs", Progress,
                                 nRead, LeftSize, recvBodySize, speed, leftTime);
                }
            }
            else if (EncodingType == EncodingGzip)
            {
                while (1)
                {
                    memset(tempBuf, 0, MAX_SIZE);
                    nRead = recvData(m_nConnectFd, tempBuf, MAX_SIZE, 0);
                    if (nRead <= 0)
                        break;
                    Response.WriteBodyBytes(tempBuf, nRead);
                    recvBodySize += nRead;
                    nTotal += nRead;
                    BodySize += nRead;
                    speed = getCurrentSpeed(tBegin, recvBodySize, 'k');
                    logger.debug("Process:--.--%%, Recv Size:%d bytes , LeftSize:--bytes, recvBodySize:%d, speed:%.2f kb/s left time:--s", nRead,
                                 recvBodySize, speed);
                }
            }
            speed = getCurrentSpeed(tBegin, recvBodySize, 'k');
            logger.info("total size:%d [%.2fKB], cost Time:%.2fs, Response Header:%d Bytes, dataSize:%d bytes speed:%.2f kb/s", nTotal, nTotal / 1000.0,
                        getSpendTime(tBegin), HeaderSize, BodySize, speed);
            return nTotal;
        }

        double getSpendTime(std::chrono::system_clock::time_point &tBegin)
        {
            auto end = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - tBegin);
            return double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
        }

        double getCurrentSpeed(std::chrono::system_clock::time_point &tBegin, ssize_t receivedBytes, char nType = 'b')
        {
            auto end = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - tBegin);
            auto costTime = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
            if (nType == 'b')
            {
                return receivedBytes / costTime;
            }
            else if (nType == 'k')
            {
                return receivedBytes / costTime / 1000;
            }
            else if (nType == 'm')
            {
                return receivedBytes / costTime / 1000 / 1000;
            }
            return receivedBytes / costTime;
        }

        ssize_t write(const char *writeBuf, ssize_t nSize)
        {
#ifndef USE_OPENSSL
            return ::send(m_nConnectFd, writeBuf, nSize, MSG_DONTWAIT);
#else
            if (m_bUseSSL)
                return SSLSend(writeBuf, nSize);
            return ::send(m_nConnectFd, writeBuf, nSize, MSG_DONTWAIT);
#endif
        }

        bool connect(const HttpUrl &url)
        {
            m_strConnectUrl = url.fullurl;
            std::string netloc = url.host;
            int nPort = url.port;
            if (url.scheme == "https")
            {
                nPort = 443;
                m_bUseSSL = true;
            }
            auto currentTime = std::chrono::system_clock::now();
            logger.info("begin to Connect %s:%d ...", netloc, nPort);
            hostent *host = gethostbyname(netloc.c_str());
            if (nullptr == host)
            {
                m_strErrorString = "can't get host ip addr yet";
                logger.error("can't get host[%s] ip addr yet", netloc);
                return false;
            }
            bool ConnectOK = false;
            int i = 0;
            logger.info("get host[%s] by name cost time %.4f s", netloc, getSpendTime(currentTime));
            std::string ipAddress;
            for (; host->h_addr_list[i]; ++i)
            {
                int AFType = host->h_addrtype;
                m_nConnectFd = socket(AFType, SOCK_STREAM, 0);
                if (m_nConnectFd == -1)
                {
                    logger.info("Create Socket error. errmsg:%s", strerror(errno));
                    m_strErrorString = "Create Socket error.";
                    m_strErrorString.append(strerror(errno));
                    continue;
                }
                sockaddr_in ServerAddress;
                memset(&ServerAddress, 0, sizeof(ServerAddress));
                ServerAddress.sin_family = AFType;
                ServerAddress.sin_port = htons(nPort);
                ServerAddress.sin_addr = *(struct in_addr *)host->h_addr_list[i];
                ipAddress = inet_ntoa(*(struct in_addr *)host->h_addr_list[i]);
                logger.info("Begin to connect address %s[%s:%d] with timeout:%d", url.fullurl, ipAddress, nPort, m_nConnectTimeout);
                // Set connect timeout
                int oldFlag = setFDnonBlock(m_nConnectFd);
                int connected = ::connect(m_nConnectFd, (struct sockaddr *)&ServerAddress, sizeof(sockaddr_in));
                if (connected == 0)
                {
                    fcntl(m_nConnectFd, F_SETFL, oldFlag);
                    ConnectOK = true;
                    logger.info("Connect to Server %s[%s:%d] immediately", url.fullurl, ipAddress, nPort);
                    break;
                }
                else if (errno != EINPROGRESS)
                {
                    logger.error("connect address %s[%s:%d] error ...", url.fullurl, ipAddress, nPort);
                    m_strErrorString = "connect address error.";
                    m_strErrorString.append(strerror(errno));
                    continue;
                }
                else
                {
                    logger.debug("unblock mode socket is connecting %s[%s:%d]...", url.fullurl, ipAddress, nPort);
                    struct timeval tm;
                    tm.tv_sec = m_nConnectTimeout;
                    tm.tv_usec = 0;
                    fd_set WriteSet;
                    FD_ZERO(&WriteSet);
                    FD_SET(m_nConnectFd, &WriteSet);
                    int res = select(m_nConnectFd + 1, nullptr, &WriteSet, nullptr, &tm);
                    if (res < 0)
                    {
                        logger.error("connect address %s[%s:%d] error ..., try next url", url.fullurl, ipAddress, nPort);
                        continue;
                    }
                    else if (res == 0)
                    {
                        logger.error("connect address %s[%s:%d] timeout[%d] ... try next url", url.fullurl, ipAddress, nPort, m_nConnectTimeout);
                        continue;
                    }
                    if (!FD_ISSET(m_nConnectFd, &WriteSet))
                    {
                        logger.warning("no Events on socket:%d found", m_nConnectFd);
                        close(m_nConnectFd);
                        continue;
                    }
                    int error = -1;
                    socklen_t len = sizeof(error);
                    if (getsockopt(m_nConnectFd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
                    {
                        logger.warning("get socket option failed");
                        close(m_nConnectFd);
                        continue;
                    }
                    if (error != 0)
                    {
                        logger.warning("connection failed after select with the error:%d", error);
                        close(m_nConnectFd);
                        continue;
                    }
                    logger.info("connection ready after select with the socket:%d", m_nConnectFd);
                    fcntl(m_nConnectFd, F_SETFL, oldFlag);
                    ConnectOK = true;
                    break;
                }
            }

            // if (::connect(m_nConnectFd, (struct sockaddr *)&ServerAddress, sizeof(sockaddr_in)) < 0) {
            //     logger.error("connect address %s[%s:%d] error ...", url.fullurl, ipAddress, nPort);
            //     continue;
            // }
            if (ConnectOK)
            {
                logger.info("connect address %s[%s:%d] Success ...", url.fullurl, ipAddress, nPort);
                if (url.scheme == "https")
                {
#if USE_OPENSSL
                    ConnectOK = SSLConnect();
                    logger.info("Switch to SSL Connection Channle... and Result is :%d, SSL_VERSION %s", ConnectOK, OPENSSL_VERSION_TEXT);
#endif
                }
                m_bConnected = ConnectOK;
            }
            else
            {
                logger.error("connect address %s error ... try times:%d", url.fullurl, i);
                close(m_nConnectFd);
            }

            return ConnectOK;
        }

        void disconnect()
        {
            ::close(m_nConnectFd);
#if USE_OPENSSL
            SSLDisConnect();
#endif
        }

        ssize_t recvData(int fd, void *buf, size_t size, int ops)
        {
#ifndef USE_OPENSSL
            return ::recv(fd, buf, size, ops);
#else
            if (m_bUseSSL)
                return SSLRecv(fd, buf, size);
            return ::recv(fd, buf, size, ops);
#endif
        }

        void setConnectTimeout(size_t nSecond)
        {
            m_nConnectTimeout = nSecond;
        }

        int setFDnonBlock(int fd)
        {
            int flag = fcntl(fd, F_GETFL, 0);
            fcntl(fd, F_SETFL, flag | O_NONBLOCK); // 保留原始状态 flag
            return flag;
        }

        bool isConnected() const
        {
            return m_bConnected;
        }

        std::string getErrorString() const
        {
            return m_strErrorString;
        }

    private:
        int m_nConnectFd;
        std::string m_strConnectUrl;
        int m_nChunkSize;
        size_t m_nConnectTimeout;
        bool m_bUseSSL;
        bool m_bSSLOpened;
        bool m_bConnected;
        std::string m_strErrorString;

#ifdef USE_OPENSSL
        typedef struct
        {
            SSL *m_ptrHandle;
            SSL_CTX *m_ptrContext;
        } SSL_Connection;
        SSL_Connection *m_pConnection;
#endif
    }; // namespace http

    class HttpClient
    {
    public:
        HttpResult Request(const std::string &reqUrl, bool bRedirect = false, bool verbose = false)
        {
            HttpUrl tempUrl(reqUrl);
            if (!m_SocketClient.connect(tempUrl))
            {
                return HttpResult(400, "", "Connect timeout.");
            }
            std::stringstream header;
            m_ReqHeader.setRequestPath(tempUrl.path);
            m_ReqHeader.set("Host", tempUrl.host);
        request:
            // 请求数据部分
            std::string HttpRequestString = m_ReqHeader.toStringHeader();
            if (verbose)
                std::cout << "* Request Header\n"
                          << m_ReqHeader << ">\n";
            ssize_t nWrite = m_SocketClient.write(HttpRequestString.c_str(), HttpRequestString.size());
            if (nWrite != HttpRequestString.size())
                return HttpResult(500, "", "Write data to server error");
            ssize_t nRead = m_SocketClient.read(m_Response);
            if(m_Response.getResponseBytesSize() == 0){
                logger.info("get response data is empty.");
            }
            else if (!m_Response.getResponseItem(ContentEncoding).empty())
            {
                m_Response.tryDecodeBody();
            }
            if (verbose)
            {
                std::cout << "* Response Body:\n"
                          << m_Response;
                std::cout << "------------------\n";
            }
            std::string itemVal = m_Response.getResponseItem(Location);
            if (!itemVal.empty() && bRedirect)
            {
                std::string RequestPath = m_ReqHeader.getRequestPath();
                if (RequestPath.rfind("/") != std::string::npos)
                    RequestPath = RequestPath.substr(0, RequestPath.rfind("/"));
                RequestPath.append("/");
                RequestPath.append(itemVal);
                m_ReqHeader.setRequestPath(RequestPath);
                logger.info("begin to redirect %s url...", RequestPath);
                m_Response.clear();
                goto request;
            }

            return HttpResult(atoi(m_Response.getResponseItem("code").c_str()), m_Response.getResponseText(), m_Response.getResponseItem("message"));
        }

        HttpResult Get(const std::string &reqUrl, bool bRedirect = false, bool verbose = false)
        {
            m_ReqHeader.setRequestType("GET");
            return Request(reqUrl, bRedirect, verbose);
        }

        HttpResult Post(const std::string &reqUrl, const std::string &strParams, bool bRedirect = false, bool verbose = false)
        {
            m_ReqHeader.setRequestType("POST");
            m_ReqHeader.setParams(strParams);
            return Request(reqUrl, bRedirect, verbose);
        }

        void SaveResultToFile(const std::string &fileName)
        {
            if (m_Response.getResponseBytesSize() == 0)
            {
                logger.warning("Download File bytes is empty. skip download");
                return;
            }
            std::ofstream fout(fileName, std::ios_base::binary);
            if (!fout.is_open())
            {
                logger.warning("Download File %s open failed. skip download", fileName);
                return;
            }
            if (m_Response.BodyIsDecoded())
            {
                fout << m_Response.getResponseText();
            }
            else
            {
                m_Response.getResponseBytes(fout);
            }
            fout.close();
        }

        void setBasicAuthUserPass(const std::string &user, const std::string &passwd)
        {
            std::stringstream ss;
            ss << user << ":" << passwd;
            std::string Base64String;
            HashUtils::EncodeBase64(ss.str(), Base64String);
            this->set(Authorization, "Basic " + Base64String);
        }

        void setUserAgent(const std::string &AgentVal)
        {
            this->set(UserAgent, AgentVal);
        }

        void setContentType(const std::string &ContentTypeVal)
        {
            this->set(ContentType, ContentTypeVal);
        }

        void setAcceptLanguage(const std::string &AcceptLanguageVal)
        {
            this->set(AcceptLanguage, AcceptLanguageVal);
        }

        void setAcceptEncoding(const std::string &AcceptEncodingVal)
        {
            this->set(AcceptEncoding, AcceptEncodingVal);
        }

        void setAccept(const std::string &AcceptVal)
        {
            this->set(Accept, AcceptVal);
        }

        void setHttpVersion(utils::HttpVersion version)
        {
            if (version == utils::HTTP_1_0)
                this->m_ReqHeader.setHttpVersion(HTTP1_0);
            else if (version == utils::HTTP_1_1)
                this->m_ReqHeader.setHttpVersion(HTTP1_1);
        }

        void set(const std::string &key, const std::string &val)
        {
            this->m_ReqHeader.set(key, val);
        }

        void setConnectTimeout(size_t nSeconds)
        {
            m_SocketClient.setConnectTimeout(nSeconds);
        }

        void setCookie(const std::string &strCookie)
        {
            m_ReqHeader.set(Cookie, strCookie);
        }

        void SaveTempFile(){
            std::ofstream fout(TempFile, std::ios::binary);
            if(!fout.is_open()) return;
            fout << "Request Header:\n" << m_ReqHeader.toStringHeader() << std::endl
                 << "<" << std::endl;
            if(m_Response.getResponseBytesSize() != 0) {
                std::string strvalue = m_Response.getResponseItem(CotentLength);
                fout << "BodyBytes:";
                if(!strvalue.empty())
                    fout << strvalue << std::endl;
                fout << "ContentEncoding:" << m_Response.getResponseItem(ContentEncoding) << std::endl;
                fout << "ContentRecvLength:" << m_Response.getResponseBytesSize() << std::endl;
                fout << "Bytes:\n" ;
                m_Response.getResponseBytes(fout);
            }
            fout.close();
            m_SocketClient.disconnect();
        }

    protected:
        HttpResource m_ReqHeader;
        HttpResource m_Response;
        std::chrono::system_clock::time_point m_ConnectTime;
        SocketClient m_SocketClient;
    };

} // namespace http