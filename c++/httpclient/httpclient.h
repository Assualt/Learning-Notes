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

#include <thread>
#ifdef USE_GURL
#include "gurl/url/gurl.h"
#else
#include "url.h"
#endif
#include "hashutils.hpp"
#include "logging.h"
using namespace std;

#ifdef USE_OPENSSL
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

#include "config.h"
#include "threadpool.h"
#include <exception>

using cstring = const std::string &;
namespace http {
typedef enum { EncodingLength, EncodingChunk, EncodingGzip, EncodingOther } Encoding;

namespace utils {

template <class T> std::string toString(const T &val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}
enum HttpVersion { HTTP_1_0, HTTP_1_1 };

static std::string _ltrim(cstring src, char ch = ' ');

std::string _rtrim(cstring src, char ch = ' ');

std::string trim(cstring src, char ch = ' ');

size_t chunkSize(cstring strChunkSize);

} // namespace utils
struct HttpUrl {
public:
    std::string scheme;
    std::string username;
    std::string password;
    std::string host;
    int         port;
    std::string path;
    std::string query;
    std::string fragment;
    std::string fullurl;
    std::string netloc;

public:
    HttpUrl(cstring url);
    HttpUrl() = default;
    void        resetUrl(cstring url);
    std::string getHostUrl() const;

private:
    void parse();
};
typedef std::vector<std::pair<std::string, std::string>> ResourceMap;
class HttpRequest {
public:
    HttpRequest();
    HttpRequest(const ResourceMap &headerMap);

    template <class T> void setHeader(cstring key, const T &val) {
        if (key == "Host")
            m_strRequestHost = utils::toString(val);
        else if (key == "Range")
            m_strRangeBytes = utils::toString(val);
        else
            m_vReqestHeader.push_back(std::pair<std::string, std::string>(key, utils::toString(val)));
    }
    void setRequestType(cstring reqType);

    void setRequestPath(cstring reqPath);

    std::string getRequestPath() const;

    void            setHttpVersion(cstring httpversion);
    void            setParams(cstring params);
    std::string     toStringHeader();
    friend ostream &operator<<(ostream &os, HttpRequest &obj);
    ResourceMap     getHeader() const;

private:
    ResourceMap m_vReqestHeader;
    std::string m_strRequestType, m_strRequestPath, m_strRequestHttpVersion;
    std::string m_strRequestParams;
    std::string m_strRequestHost;
    std::string m_strRangeBytes;
};

class HttpResponse {
public:
    HttpResponse();
    void               setBody(cstring key, cstring val);
    std::string        getResponseText() const;
    void               getResponseBytes(std::ostream &os);
    size_t             getResponseBytesSize() const;
    size_t             getResponseDecodeBytesSize() const;
    std::string        getResponseItem(cstring key);
    std::string        getCookie();
    void               WriteBodyBytes(const char *buf, size_t nBytes);
    void               WriteBodyByte(char ch);
    void               tryDecodeBody();
    bool               BodyIsDecoded() const;
    const ResourceMap &GetResponse() const;
    friend ostream &   operator<<(ostream &os, HttpResponse &obj);
    void               clear();

private:
    ssize_t        m_nBodyBytes;
    ssize_t        m_nBodyDecodeBytes;
    bool           m_bDecodeBodyStatus;
    ResourceMap    m_vResponseBody;
    std::string    m_strResponseText;
    MyStringBuffer m_ResponseBuffer;
};

struct HttpResult {
private:
    int         m_nStatusCode;
    std::string m_strText;
    std::string m_strRelayError;

public:
    HttpResult()
        : m_nStatusCode(200) {
    }
    HttpResult(int code, cstring text, cstring relayError)
        : m_nStatusCode(code)
        , m_strText(text)
        , m_strRelayError(relayError) {
    }

    int status_code() const {
        return m_nStatusCode;
    }

    std::string text() const {
        return m_strText;
    }

    std::string error() const {
        return m_strRelayError;
    }
};

enum HTTP_TYPE { TYPE_GET, TYPE_POST, TYPE_DELETE, TYPE_PUT };

enum CHUNK_STATE { CHUNK_BEGIN, CHUNK_PROCESS_1, CHUNK_PROCESS_2, CHUNK_END };

#define MAX_SIZE 32768
class SocketClient {
public:
    SocketClient();
    ~SocketClient();

private:
#ifdef USE_OPENSSL
    void    initSSL();
    bool    SSLConnect();
    void    printSSLConnection(SSL *ssl);
    void    SSLDisConnect();
    ssize_t SSLRecv(int fd, void *buf, ssize_t size);

    ssize_t SSLSend(const void *buf, ssize_t size);
#endif
    int      RecvChunkData(const char *buffer, size_t &nPos, int size, HttpResponse &Response, CHUNK_STATE &ChunkState, size_t &ChunkSize, ssize_t &AllChunkBodySize, size_t &blockCount);
    Encoding ParseHeader(const char *buffer, ssize_t size, HttpResponse &Response, ssize_t &HeaderSize, ssize_t &recvBodySize, ssize_t &LeftBodySize, ssize_t &BodySizeInResponse);
    void     ParseHeaderLine(cstring line, std::string &key, std::string &val);
    void     ParseFirstLine(cstring line, std::string &HttpVersion, int &StatusCode, std::string &StatusMessage);

public:
    ssize_t     read(HttpResponse &Response, cstring outfile);
    std::string getChunkString(Encoding nType);

    static double getSpendTime(std::chrono::system_clock::time_point &tBegin);
    static double getCurrentSpeed(std::chrono::system_clock::time_point &tBegin, ssize_t receivedBytes, char nType = 'b');
    ssize_t       write(const char *writeBuf, ssize_t nSize);
    bool          connect(cstring url);
    bool          connect(const HttpUrl &url);
    void          disconnect();
    int           recvData(int fd, void *buf, size_t size, int ops);

    void setConnectTimeout(size_t nSecond);
    int  setFDnonBlock(int fd);

    bool        isConnected() const;
    std::string getErrorString() const;
    void        setRequestType(cstring RequestType);
    std::string getRequestType() const;

    int getConnectedFd() const;

private:
    int         m_nConnectFd;
    std::string m_strConnectUrl;
    int         m_nChunkSize;
    size_t      m_nConnectTimeout;
    bool        m_bUseSSL;
    bool        m_bSSLOpened;
    bool        m_bConnected;
    std::string m_strErrorString;
    std::string m_strRequestType;

#ifdef USE_OPENSSL
    typedef struct {
        SSL *    m_ptrHandle;
        SSL_CTX *m_ptrContext;
    } SSL_Connection;
    SSL_Connection *m_pConnection;
#endif

}; // namespace http
class HttpClient {

protected:
    HttpRequest                           m_ReqHeader;
    HttpResponse                          m_Response;
    std::chrono::system_clock::time_point m_ConnectTime;
    SocketClient                          m_SocketClient;
    std::string                           m_strRequestHost;
    std::threadpool                       m_DownloadsThreadPool;
    std::string                           m_strOutputFile;

public:
    class DownloadThreadMgr {
    public:
        struct RangeInfo {
            RangeInfo() = default;
            RangeInfo(ssize_t begin, ssize_t end);
            ssize_t     nBegin;
            ssize_t     nEnd;
            std::string strRequstHeader;
            std::string strTmpFile;
            std::string strReqUrl;
            std::string strRange() const;
            void        setRequestHeader(cstring header);
            void        setThreadTmpFile(cstring tmpFile);
            void        setReqUrl(cstring strUrl);
        };
        DownloadThreadMgr(ssize_t FileLength, int nThreadCount, cstring reqUrl, HttpRequest &RequestHeader, cstring strDownloadPath);

        bool startThread();

    protected:
        ssize_t                                        m_nFileLength;
        int                                            m_nThreadCount;
        std::vector<std::pair<std::string, RangeInfo>> m_vthreadsInfo;
        ssize_t                                        m_nReceivedBytes;
        std::string                                    m_strDownLoadFile;
        std::mutex                                     m_lock;
    };

    void                    setOutputFile(cstring file);
    HttpResult              Request(cstring reqUrl, bool bRedirect = false, bool verbose = false);
    std::string             GetRedirectLocation(cstring strLocation, cstring RequestPath);
    HttpResult              Get(cstring reqUrl, bool bRedirect = false, bool verbose = false);
    HttpResult              Post(cstring reqUrl, cstring strParams, bool bRedirect = false, bool verbose = false);
    HttpResult              Head(cstring reqUrl, bool verbose = false);
    void                    SaveResultToFile(cstring fileName);
    void                    setBasicAuthUserPass(cstring user, cstring passwd);
    void                    setUserAgent(cstring AgentVal);
    void                    setContentType(cstring ContentTypeVal);
    void                    setAcceptLanguage(cstring AcceptLanguageVal);
    void                    setAcceptEncoding(cstring AcceptEncodingVal);
    void                    setAccept(cstring AcceptVal);
    void                    setHttpVersion(utils::HttpVersion version);
    template <class T> void setHeader(cstring key, const T &val) {
        this->m_ReqHeader.setHeader(key, val);
    }

    void        setConnectTimeout(size_t nSeconds);
    void        setCookie(cstring strCookie);
    void        SaveTempFile();
    void        setReferer(cstring strReferer);
    std::string getCookie();
    void        DownloadFile(cstring reqUrl, cstring downloadPath, int nThreadCount = 10, bool verbose = false);
};

} // namespace http
