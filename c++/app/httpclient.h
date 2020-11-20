#include <iostream>
#include <map>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netdb.h> /* gethostbyname */
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sstream>
#include <streambuf>
#include "url/gurl.h"
#include "logging.h"
#include "hashutils.h"
using namespace std;

namespace http
{
    namespace utils
    {
        std::string _ltrim(const std::string &src, char ch = ' ')
        {
            std::string temp = src;
            std::string::iterator p = std::find_if(temp.begin(), temp.end(), [&ch](char c) { return ch != c; });
            temp.erase(temp.begin(), p);
            return temp;
        }
        std::string _rtrim(const std::string &src, char ch = ' ')
        {
            string temp = src;
            string::reverse_iterator p = find_if(temp.rbegin(), temp.rend(), [&ch](char c) { return ch != c; });
            temp.erase(p.base(), temp.end());
            return temp;
        }
        std::string trim(const std::string &src, char ch = ' ')
        {
            return _rtrim(_ltrim(src, ch), ch);
        }
    } // namespace utils
    struct RequestHeader;
    struct Url
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
        Url(const std::string &url) : fullurl(url), scheme("http"), port(80)
        {
            parse();
        }
        Url() = default;

    private:
        void parse()
        {
            if (fullurl.find("://") == std::string::npos)
                fullurl = "http://" + fullurl;
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
            netloc = host + ":" + to_string(port);
        }
    };

    struct MyStringBuffer : public stringbuf
    {
    public:
        void seekReadPos(ssize_t nPos)
        {
            stringbuf::seekpos(nPos, std::ios_base::in);
        }
        string toString()
        {
            string temp;
            seekReadPos(0);
            do
            {
                temp.push_back(this->sgetc());
            } while (this->snextc() != EOF);
            seekReadPos(0);
            return temp;
        }
        void seekWritePos(ssize_t nPos)
        {
            stringbuf::seekpos(nPos, std::ios_base::out);
        }
    };

    struct RequestHeader
    {
    public:
        RequestHeader() : m_nBodyBytes(0), m_nBodyDecodeBytes(0)
        {
        }
        void set(const std::string &key, const std::string &val) { headers.push_back(std::pair<std::string, std::string>(key, val)); }
        void setBody(const std::string &key, const std::string &val) { ResponseBody.push_back(std::pair<std::string, std::string>(key, val)); }
        std::string getResponseText() const { return m_strResponseText; }
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
        void setRequestType(const std::string &reqType) { m_strRequestType = reqType; }
        void setRequestPath(const std::string &reqPath) { m_strRequestPath = reqPath; }
        void setHttpVersion(const std::string &httpversion) { m_strRequestHttpVersion = httpversion; }
        void setParams(const std::string &params) { m_strRequestParams = params; }
        size_t getResponseBytesSize() const { return m_nBodyBytes; }
        std::string toStringHeader()
        {
            stringstream ss;
            ss << m_strRequestType << " " << m_strRequestPath << " " << m_strRequestHttpVersion << "\r\n";
            for (auto &item : headers)
            {
                ss << item.first << ": " << item.second << "\r\n";
            }
            ss << "\r\n";
            ss << m_strRequestParams << "\r\n";
            return ss.str();
        }
        friend ostream &operator<<(ostream &os, RequestHeader &obj)
        {
            if (!obj.headers.empty())
                os << "> " << obj.m_strRequestType << " " << obj.m_strRequestPath << " " << obj.m_strRequestHttpVersion << "\r\n";
            for (auto &item : obj.headers)
            {
                os << "> " << item.first << ": " << item.second << "\r\n";
            }
            for (auto &item : obj.ResponseBody)
            {
                os << "< " << item.first << ": " << item.second << "\r\n";
            }
            if (!obj.headers.empty() && !obj.m_strRequestParams.empty())
            {
                os << "\r\n";
                os << obj.m_strRequestParams << "\r\n";
            }
            if (!obj.m_strResponseText.empty())
            {
                os << "<\r\n";
                os << obj.m_strResponseText << "\r\n";
            }
            os << "\r\n"
               << "Recv Body Size:" << obj.m_nBodyBytes << " Decode Bytes:"
               << obj.m_nBodyDecodeBytes
               << std::endl;
            return os;
        }
        std::string getResponseItem(const std::string &key)
        {
            for (auto &item : ResponseBody)
            {
                if (item.first == key)
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
            if (this->getResponseItem("Content-Encoding") == "gzip")
            {
                logger.info("Try Decode Bytes %d for gzip.", m_nBodyBytes);
                std::stringstream strstring;
                char *WriteBuffer = new char[MAX_BUF_SIZE];
                bool DecodeError = false;
                while (1)
                {
                    memset(WriteBuffer, 0, MAX_BUF_SIZE);
                    std::streamsize nReadSize = m_ResponseBuffer.sgetn(WriteBuffer, MAX_BUF_SIZE);
                    if (nReadSize == 0)
                        break;
                    int nSize = HashUtils::GZipDEcompress(WriteBuffer, nReadSize, strstring);
                    if (nSize < 0)
                    {
                        DecodeError = true;
                        break;
                    }
                    logger.info("decode bytes %d", nSize);
                }
                if (!DecodeError)
                {
                    m_strResponseText = strstring.str();
                    m_nBodyDecodeBytes = m_strResponseText.size();
                }else{
                    logger.info("Decode Error..");
                }
            }
        }

    private:
        std::vector<std::pair<std::string, std::string>> headers;
        std::vector<std::pair<std::string, std::string>> ResponseBody;
        std::string m_strRequestType, m_strRequestPath, m_strRequestHttpVersion;
        std::string m_strRequestParams, m_strResponseText;
        ssize_t m_nBodyBytes, m_nBodyDecodeBytes;
        MyStringBuffer m_ResponseBuffer;
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
        int status_code() const { return m_nStatusCode; }
        std::string text() const { return m_strText; }
        std::string error() const { return m_strRelayError; }
    };

    enum HTTP_TYPE
    {
        TYPE_GET,
        TYPE_POST,
        TYPE_DELETE,
        TYPE_PUT
    };
#define MAX_SIZE 8192
    class SocketClient
    {
    public:
        SocketClient() = default;
        ~SocketClient()
        {
            disconnect();
        }

    private:
        bool parse_Header(const char *buffer, ssize_t size, RequestHeader &header, ssize_t &HeaderSize, ssize_t &BodySize, ssize_t &LeftBodySize)
        {
            std::string strKey, strVal, strLine;
            int LineCnt = 0;
            bool bFindBody = false;
            ssize_t BodySizeInResponse = 0;
            int LengthType = 0; // 0: Content-Length 1: trunked 2: other way
            int nCnt = 0;
            MyStringBuffer trunkedBuffer;
            int trunkState = 0;
            int trunkSize = 0, trunkSize1 = 0;
            std::string strTrunkSize;

            for (size_t i = 0; i < size - 1;)
            {
                if (buffer[i] == '\r' && buffer[i + 1] == '\n' && !bFindBody)
                {
                    if (LineCnt == 0)
                    {
                        std::string HttpVersion, statusMessage;
                        int StatusCode;
                        parse_FirstLine(strLine, HttpVersion, StatusCode, statusMessage);
                        logger.info(" http:%s, Code:%d, StatusMessage:%s", HttpVersion, StatusCode, statusMessage);
                        LineCnt++;
                    }
                    else if (!strLine.empty())
                    {
                        parse_Line(strLine, strKey, strVal);
                        strKey = utils::trim(strKey);
                        strVal = utils::trim(strVal);
                        logger.debug("Line: %s->%s", strKey, strVal);
                        if (strKey == "Content-Length")
                        {
                            BodySizeInResponse = atoi(strVal.c_str());
                            LengthType = 0;
                        }
                        else if (strKey == "Transfer-Encoding" && strVal == "chunked")
                        {
                            LengthType = 1;
                        }
                        else if (BodySizeInResponse == 0 && LengthType != 1)
                        {
                            LengthType = 2;
                        }
                        header.setBody(strKey, strVal);
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
                {
                    strLine.push_back(buffer[i]);
                    i++;
                }
                else if (buffer[i] == '\r' && bFindBody)
                {
                    if (LengthType != 1)
                    {
                        header.WriteBodyByte(buffer[i]);
                        BodySize += 1;
                        i += 1;
                    }
                    else
                    {
                        if (trunkState == 0) // Buffer Size 100d\r\n
                        {
                            trunkSize = atoi(strTrunkSize.c_str());
                            trunkSize1 += trunkSize;
                            trunkState += 1;
                            i += 1;
                            logger.info("Recv Trunked Message Size: %d trunkState:%d buffer[i] %d", atoi(trunkedBuffer.toString().c_str()), trunkState, buffer[i]);
                            strTrunkSize.clear();
                        }
                        else if (trunkState == 1)
                        {
                            logger.info("current state is here");
                        }
                        else if (trunkState == 2)
                        { //blank \r\n
                            i += 1;
                            trunkState = 0;
                        }
                        i += 1;
                    }
                }
                else
                {
                    if (LengthType != 1)
                    {
                        header.WriteBodyByte(buffer[i]);
                        BodySize += 1;
                    }
                    else
                    {
                        if (trunkState == 0)
                        {
                            strTrunkSize.push_back(buffer[i]);
                        }
                        else if (trunkState == 1)
                        {
                            if (trunkSize--)
                            {
                                trunkedBuffer.sputc(buffer[i] & 0xFF);
                                // logger.debug("append byte %d to trunkedBuffer leftSize:%d", (int)buffer[i], trunkSize);
                                printf("append byte:%x trunkSize:%d\n", buffer[i] & 0xFF, trunkSize);
                            }
                            else
                            {
                                logger.debug("trunked buffer End");
                                trunkState = 2;
                            }
                        }
                    }
                    // logger.info("i:%d write bytes:%s ", i, buffer[i]);
                    i++;
                }
            }
            // append the last bit
            header.WriteBodyByte(buffer[size - 1]);
            BodySize += 1;
            if (LengthType == 0 && BodySizeInResponse != 0)
            {
                LeftBodySize = BodySizeInResponse - BodySize;
                logger.info("Header Size:%d recvBodySize:%d totalSize:%d BodySizeInResponse:%d LeftBodySize:%d", HeaderSize, BodySize, size, BodySizeInResponse, LeftBodySize);
            }
            else if (LengthType == 1)
            {
                char *tempBuf = new char[trunkSize1];
                size_t nRead = trunkedBuffer.sgetn(tempBuf, trunkSize1);
                if (nRead == trunkSize1)
                {
                    logger.info("success append:%d bytes trunk data", nRead);
                    header.WriteBodyBytes(tempBuf, nRead);
                }
                else
                {
                    header.WriteBodyBytes(tempBuf, nRead);
                    LeftBodySize = trunkSize1;
                    logger.info("nRead:%d, trunkSize:%d ", nRead, trunkSize1);
                }
                delete[] tempBuf;
                logger.info("Use Trunked data.");
            }

            std::cout << buffer << std::endl;

            return LengthType;
        }
        void parse_Line(const std::string &line, std::string &key, std::string &val)
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

        void parse_FirstLine(const std::string &line, std::string &HttpVersion, int &StatusCode, std::string &StatusMessage)
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
        ssize_t read(RequestHeader &response)
        {
            char tempBuf[MAX_SIZE];
            ssize_t headerSize = 0, bodySize = 0;
            ssize_t recvBodySize = 0;
            ssize_t nRead = ::recv(m_nConnectFd, tempBuf, MAX_SIZE, 0);
            if (nRead < 0)
            {
                return 0;
            }
            size_t nTotal = nRead;
            // GET Content Length From first Buffer
            std::stringstream ss(std::string(tempBuf, nRead));
            std::string temp;
            bool bFindBlankLine = false;
            bool bFindChunked = false;
            ssize_t LeftSize;
            int LengthType = parse_Header(tempBuf, nRead, response, headerSize, bodySize, LeftSize);
            if (LeftSize <= 0 && LengthType == 0)
            {
                logger.info("Request Finished.");
            }
            else if (LengthType == 2)
            {
                while (1)
                {
                    if (LeftSize > 0) // means 当前还未接收完毕
                    {
                        memset(tempBuf, 0, MAX_SIZE);
                        nRead = ::recv(m_nConnectFd, tempBuf, LeftSize, 0);
                        if (nRead > 0)
                        {
                            response.WriteBodyBytes(tempBuf, nRead);
                            logger.debug("Recv Data:%d ", nRead);
                        }
                    }
                    else
                    {
                        memset(tempBuf, 0, MAX_SIZE);
                        nRead = ::recv(m_nConnectFd, tempBuf, LeftSize, 0);
                        if (nRead <= 0)
                            break;
                        logger.debug("tempBuf %d", nRead);
                    }
                }
            }
            else
            {
                while (LeftSize > 0)
                {
                    memset(tempBuf, 0, MAX_SIZE);
                    nRead = ::recv(m_nConnectFd, tempBuf, MAX_SIZE, 0);
                    if (nRead <= 0)
                        break;
                    response.WriteBodyBytes(tempBuf, nRead);
                    recvBodySize += nRead;
                    nTotal += nRead;
                    LeftSize -= nRead;
                    bodySize += nRead;
                    logger.debug("Recv Size:%d bytes , LeftSize:%d bytes, recvBodySize:%d", nRead, LeftSize, recvBodySize);
                }
            }
            logger.debug("total size:%d, headerSize:%d, bodySize:%d", nTotal, headerSize, bodySize);
            return nTotal;
        }
        ssize_t write(const char *writeBuf, ssize_t nSize)
        {
            return ::write(m_nConnectFd, writeBuf, nSize);
        }
        bool connect(const Url &url)
        {
            m_strConnectUrl = url.fullurl;
            std::string netloc = url.host;
            int nPort = url.port;
            if (url.scheme == "https")
                nPort = 443;
            logger.info("begin to Connect %s:%d ...", netloc, nPort);
            hostent *host = gethostbyname(netloc.c_str());
            if (nullptr == host)
                return false;
            bool ConnectOK = false;
            for (int i = 0; host->h_addr_list[i]; ++i)
            {
                int AFType = host->h_addrtype;
                m_nConnectFd = socket(AFType, SOCK_STREAM, 0);
                if (m_nConnectFd == -1)
                    continue;
                sockaddr_in ServerAddress;
                memset(&ServerAddress, 0, sizeof(ServerAddress));
                ServerAddress.sin_family = AFType;
                ServerAddress.sin_port = htons(nPort);
                ServerAddress.sin_addr = *(struct in_addr *)host->h_addr_list[i];
                std::string ipAddress = inet_ntoa(*(struct in_addr *)host->h_addr_list[i]);
                if (::connect(m_nConnectFd, (struct sockaddr *)&ServerAddress, sizeof(sockaddr_in)) < 0)
                {
                    logger.error("connect address %s[%s:%d] error ...", url.fullurl, ipAddress, nPort);
                    continue;
                }
                ConnectOK = true;
                logger.info("connect address %s[%s:%d] Success ...", url.fullurl, ipAddress, nPort);
                break;
            }
            return ConnectOK;
        }
        bool disconnect()
        {
            ::close(m_nConnectFd);
        }
        int setnonblocking(int fd)
        { //设置套接字为非阻塞套接字
            int old_option = fcntl(fd, F_GETFL);
            int new_option = old_option | O_NONBLOCK;
            fcntl(fd, F_SETFL, new_option);
            return old_option;
        }

    private:
        int m_nConnectFd;
        std::string m_strConnectUrl;
    };

    class HttpClient
    {
    public:
        HttpResult Request(const std::string &reqUrl, HTTP_TYPE ntype, bool verbose = false)
        {
            Url tempUrl(reqUrl);
            SocketClient client;
            if (!client.connect(tempUrl))
            {
                return HttpResult(400, "", "Connect TimeOut.");
            }
            std::stringstream header;
            switch (ntype)
            {
            case TYPE_POST:
                m_ReqHeader.setRequestType("POST");
                break;
            case TYPE_PUT:
                m_ReqHeader.setRequestType("PUT");
                break;
            case TYPE_DELETE:
                m_ReqHeader.setRequestType("DELETE");
                break;
            case TYPE_GET:
            default:
                m_ReqHeader.setRequestType("GET");
                break;
            }
            m_ReqHeader.setRequestPath(tempUrl.path);

            m_ReqHeader.set("Host", tempUrl.host);

            // 请求数据部分
            std::string RequestHeaderString = m_ReqHeader.toStringHeader();
            if (verbose)
            {
                std::cout << "* Request Header\n"
                          << m_ReqHeader << ">\n";
            }
            ssize_t nWrite = client.write(RequestHeaderString.c_str(), RequestHeaderString.size());
            if (nWrite != RequestHeaderString.size())
                return HttpResult(500, "", "Write data to server error");

            ssize_t nRead = client.read(m_Response);

            if (m_Response.getResponseItem("Content-Encoding") == "gzip")
            {
                m_Response.tryDecodeBody();
            }
            if (verbose)
            {
                std::cout << "* Response Body:\n"
                          << m_Response;
                std::cout << "------------------\n";
            }
            return HttpResult(atoi(m_Response.getResponseItem("code").c_str()), m_Response.getResponseText(), m_Response.getResponseItem("message"));
        }
        HttpResult Get(const std::string &reqUrl, bool verbose = false)
        {
            return Request(reqUrl, TYPE_GET, verbose);
        }
        HttpResult Post(const std::string &reqUrl, bool verbose = false)
        {
            return Request(reqUrl, TYPE_POST, verbose);
        }

        void DownloadToFile(const std::string &fileName)
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
            if (m_Response.getResponseItem("Accept-Ranges") == "bytes")
            {
                fout << m_Response.getResponseText();
            }
            else
            {
                m_Response.getResponseBytes(fout);
            }
            fout.close();
        }

        void setAuthUserPass(const std::string &user, const std::string &passwd)
        {
            std::stringstream ss;
            ss << user << ":" << passwd;
            std::string Base64String;
            HashUtils::EncodeBase64(ss.str(), Base64String);
            this->set("Authorization", "Basic " + Base64String);
        }

        void setUserAgent(const std::string &AgentVal)
        {
            this->set("User-Agent", AgentVal);
        }

        void setContentType(const std::string &ContentTypeVal)
        {
            this->set("Content-Type", ContentTypeVal);
        }

        void setAcceptLanguage(const std::string &AcceptLanguageVal)
        {
            this->set("Accept-Language", AcceptLanguageVal);
        }

        void setAcceptEncoding(const std::string &AcceptEncodingVal)
        {
            this->set("Accept-Encoding", AcceptEncodingVal);
        }

        void setAccept(const std::string &AcceptVal)
        {
            this->set("Accept", AcceptVal);
        }

        void setHttpVersion(const std::string &httpversionVal)
        {
            this->m_ReqHeader.setHttpVersion(httpversionVal);
        }

        void set(const std::string &key, const std::string &val)
        {
            this->m_ReqHeader.set(key, val);
        }

    protected:
        RequestHeader m_ReqHeader;
        RequestHeader m_Response;
    };
} // namespace http