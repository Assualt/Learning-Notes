#include "httpserver.h"
#include "hashutils.hpp"
#include <thread>
#define BUF_SIZE 32768
namespace http {

HttpRequest::HttpRequest() {
    m_strRequestParams = "";
    m_strRequestHost   = "";
    m_strRangeBytes    = "";
}

void HttpRequest::setRequestType(const std::string &reqType) {
    m_strRequestType = reqType;
}
std::string HttpRequest::getRequestType() const {
    return m_strRequestType;
}

void HttpRequest::setRequestPath(const std::string &reqPath) {
    m_strRequestPath = reqPath;
}

std::string HttpRequest::getRequestPath() const {
    return m_strRequestPath;
}

void HttpRequest::setHttpVersion(const std::string &httpversion) {
    m_strRequestHttpVersion = httpversion;
}

std::string HttpRequest::getHttpVersion() const {
    return m_strRequestHttpVersion;
}

void HttpRequest::setPostParams(const std::string &params) {
    m_strRequestParams = params;
}

std::string HttpRequest::getPostParams() const {
    return m_strRequestParams; // = params;
}

std::string HttpRequest::toStringHeader() {
    stringstream ss;
    ss << m_strRequestType << " " << m_strRequestPath << " " << m_strRequestHttpVersion << CTRL;
    for (auto &item : m_vReqestHeader)
        ss << item.first << ": " << item.second << CTRL;
    if (!m_strRangeBytes.empty())
        ss << "Range: " << m_strRangeBytes << CTRL;
    ss << "Host: " << m_strRequestHost << CTRL;

    if (!m_strRequestParams.empty())
        ss << CTRL << m_strRequestParams;
    ss << CTRL;
    return ss.str();
}

void HttpResponse::setStatusMessage(int statusCode, const std::string &HttpVersion, const std::string &message) {
    m_nstatusCode    = statusCode;
    m_strHttpVersion = HttpVersion;
    m_strMessage     = message;
}

std::string HttpResponse::toResponseHeader() {
    stringstream ss;
    ss << m_strHttpVersion << " " << m_nstatusCode << " " << m_strMessage << CTRL;
    for (auto &item : m_vResponseHeader)
        ss << item.first << ": " << item.second << CTRL;
    if (!m_strBodyString.empty())
        ss << CTRL << m_strBodyString << CTRL;
    ss << CTRL;
    return ss.str();
}

void HttpResponse::setBodyString(const std::string &strBodyString) {
    m_strBodyString = strBodyString;
}

ClientThread::ClientThread(int serverFd, int clientFd)
    : m_nThreadserverFd(serverFd)
    , m_nThreadClientFd(clientFd) {
}

void ClientThread::parseHeader(HttpRequest &request) {
    char           temp[ BUF_SIZE ];
    bool           bFindBody = false;
    std::string    strLine, strKey, strVal;
    int            LineCnt = 0;
    int            BodySizeInRequest, recvSize = 0;
    Encoding       EncodingType;
    int            HeaderSize = 0;
    std::string    HttpVersion, RequestType, RequestPath;
    MyStringBuffer mybuf;
    while (!bFindBody) {
        int nRead = recvData(m_nThreadClientFd, temp, BUF_SIZE, 0);
        if (nRead <= 0)
            return;
        for (size_t i = 0; i < nRead;) {
            if (i + 1 < nRead && temp[ i ] == '\r' && temp[ i + 1 ] == '\n' && !bFindBody) {
                if (LineCnt == 0) {
                    ParseFirstLine(strLine, HttpVersion, RequestPath, RequestType);
                    request.setRequestType(RequestType);
                    request.setRequestPath(RequestPath);
                    request.setHttpVersion(HttpVersion);
                    logger.info("httpversion:%s requesttype:%s requestpath:%s", HttpVersion, RequestType, RequestPath);
                    LineCnt++;
                } else if (!strLine.empty()) {
                    ParseHeaderLine(strLine, strKey, strVal);
                    strKey = utils::trim(strKey);
                    strVal = utils::trim(strVal);
                    logger.debug("Find Response Body Header: %s->%s", strKey, strVal);
                    if (strcasecmp(strKey.c_str(), ContentLength) == 0) {
                        BodySizeInRequest = atoi(strVal.c_str());
                        EncodingType      = EncodingLength;
                    } else if (strcasecmp(strKey.c_str(), TransferEncoding) == 0 && strcasecmp(strVal.c_str(), "chunked") == 0) {
                        EncodingType = EncodingChunk;
                    }
                    request.setHeader(strKey, strVal);
                } else if (strLine.empty()) {
                    bFindBody = true;
                }
                i += 2;
                HeaderSize += strLine.size() + 2;
                strLine.clear();
            } else if (!bFindBody) { // Header
                strLine.push_back(temp[ i ]);
                i++;
            } else if (strcasecmp(RequestType.c_str(), "get") == 0) {
                break;
            } else if (strcasecmp(RequestType.c_str(), "post") == 0 && BodySizeInRequest != 0) {
                mybuf.sputc(temp[ i ]);
                recvSize++;
                i++;
            }
        }
    }
    if (strcasecmp(RequestType.c_str(), "post") == 0)
        request.setPostParams(mybuf.toString());
}

void ClientThread::ParseHeaderLine(const std::string &line, std::string &key, std::string &val) {
    int nBlankCnt = 0;
    key.clear();
    val.clear();
    for (size_t i = 0; i < line.size(); i++) {
        if (line[ i ] == ':')
            if(nBlankCnt == 0)
                nBlankCnt = 1;
            else
                val.push_back(line[i]);
        else if (nBlankCnt == 0) {
            key.push_back(line[ i ]);
        } else if (nBlankCnt == 1) {
            val.push_back(line[ i ]);
        }
    }
}

void ClientThread::ParseFirstLine(const std::string &line, std::string &HttpVersion, std::string &RequestPath, std::string &RequestType) {
    int nBlankCnt = 0;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[ i ] == ' ') {
            if (nBlankCnt != 2)
                nBlankCnt++;
            else
                HttpVersion.push_back(line[ i ]);
        } else if (nBlankCnt == 0) {
            RequestType.push_back(line[ i ]);
        } else if (nBlankCnt == 1) {
            RequestPath.push_back(line[ i ]);
        } else if (nBlankCnt == 2) {
            HttpVersion.push_back(line[ i ]);
        }
    }
}
void ClientThread::handRequest(const RequestMapper &handerMapping) {
    HttpRequest  request;
    HttpResponse response;
    char         tempBuf[ BUF_SIZE ];
    parseHeader(request);
    logger.info("request type:%s requst path:%s httpversion:%s", request.getRequestType(), request.getRequestPath(), request.getHttpVersion());
    // if (handerMapping.find(request.getRequestPath()) != handerMapping.end()) {
    //     auto c = handerMapping.at(request.getRequestPath());
    //     c(request, response);
    //     writeResponse(response);
    // } else {
    //     logger.info("request path:%s 404 not found", request.getRequestPath());
    // }
    if(handerMapping.)

}
ssize_t ClientThread::writeResponse(HttpResponse &response) {
    std::string responseHeader = response.toResponseHeader();
    logger.info("Response Header:\n%s", responseHeader);
    return writeData(m_nThreadClientFd, (void *)responseHeader.c_str(), responseHeader.size(), 0);
}

int ClientThread::recvData(int fd, void *buf, size_t n, int ops) {
    return ::recv(fd, buf, n, ops);
}
int ClientThread::writeData(int fd, void *buf, size_t n, int ops) {
    return ::send(fd, buf, n, ops);
}

HttpServer::HttpServer(const std::string &strServerName, const std::string &strServerIP, const std::string &strServerDescription, int nPort)
    : m_strServerName(strServerName)
    , m_strServerIP(strServerIP)
    , m_strServerDescription(strServerDescription)
    , m_nPort(nPort)
    , m_nMaxListenClients(20)
    , m_nServerFd(-1)
    , m_nEpollTimeOut(10) {
}

bool HttpServer::ExecForever() {
    int ret = 0;
    // 1.创建server socket 用作监听使用，使用TCP可靠的连接和 IPV4的形式进行事件进行监听，协议使用IPV4
    m_nServerFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nServerFd == -1) {
        logger.warning("create socket error with erros:%s", strerror(errno));
        return false;
    }
    // 2.server端进行bind 并初始化server的地址结构体, 注意字节序的转化
    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(m_nPort);
    if ((ret = bind(m_nServerFd, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1) {
        logger.warning("bind error with erros:%s", strerror(errno));
        return false;
    }
    // 3.server端进行监听
    if ((ret = listen(m_nServerFd, m_nMaxListenClients)) == -1) {
        logger.warning("listen error with erros:%s", strerror(errno));
        return false;
    }

    logger.info("httpserver:%s listening at %s:%d ....", m_strServerName, m_strServerIP, m_nPort);
    // 1. epoll wait
    int epoll_fd = epoll_create(1);
    // 2.定义并填充epoll_event,和监听连接接的server的所有epoll_events事件数组
    struct epoll_event event;
    event.events                           = EPOLLIN;
    event.data.fd                          = m_nServerFd; //服务器绑定的fd
    struct epoll_event *event_client_array = new epoll_event[ m_nMaxListenClients ];
    // 3.注册事件,将当前的epoll_fd注册到内核中去用于事件的监听
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, m_nServerFd, &event);
    // 4. 进入事件等待
    while (true) {
        //设置最大的监听的数量,以及EPOLL 的超时时间
        int ready_count = epoll_wait(epoll_fd, event_client_array, m_nMaxListenClients, m_nEpollTimeOut);
        // 5.从就绪的event里面进行accept,用于创建链接到客户端
        for (int i = 0; i < ready_count; i++) {
            socklen_t          addr_len;
            struct sockaddr_in client_addr;
            int                client_fd;
            if (event_client_array[ i ].data.fd == m_nServerFd) { //确定为连接服务器的请求
                int client_fd = accept(m_nServerFd, (struct sockaddr *)&client_addr, &addr_len);
                // 6.把接收到client的fd 添加到epoll的监听事件中去
                event.events  = EPOLLIN;
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, m_nServerFd, &event);
                logger.info("recv client:%d from address %s ", client_fd, inet_ntoa(client_addr.sin_addr));
                if (client_fd != -1) {
                    ClientThread clientThread(m_nServerFd, client_fd);
                    std::thread  tempThread(&ClientThread::handRequest, clientThread, m_mapper);
                    tempThread.join();
                } else {
                    logger.info("client fd is -1");
                }
            } else if (event_client_array[ i ].events & EPOLLIN) {
                // 7.进行数据的交流
                //可创建一个线程与此fd进行数据的处理操作
                //线程处理完毕之后就直接可以直接从监听的的fd中移除掉
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, m_nServerFd, &event);
                close(client_fd);
            }
        }
    }
    // 8.关闭和释放支援
    close(epoll_fd);
    close(m_nServerFd);
    delete[] event_client_array;

    return false;
}

bool HttpServer::addRequestMapping(const std::string &path, Func&& F){
    if (m_mHandleMapping.find(path) != m_mHandleMapping.end()) {
        logger.info("add Request path:%s is exists. ignore it", path);
        return false;
    }
    m_mHandleMapping.insert(std::pair<std::string, Func>(path, F));
    return true;
}

bool HttpServer::loadHttpConfig(const std::string &strHttpServerConfig){
    bool ret = m_mConfig.loadConfig(strHttpServerConfig);
    return ret;
}

bool HttpServer::HttpConfig::loadConfig(const std::string &strConfigFilePath){
    if(access(strConfigFilePath.c_str(), F_OK) == -1){
        logger.warning("load default config %s failed.", strConfigFilePath);
        return false;
    }
    return true;
}


} // namespace http