#include "httpserver.h"
#include <thread>
#define BUF_SIZE 32768
namespace http {
tlog::logImpl HttpServer::ServerLog;
RequestMapper::Key::Key(const std::string &pattern, const std::string &method, bool needval) : pattern(pattern), method(method), needval(needval) {
    auto itemVector = utils::split(pattern, '/');
    if (needval) {
        int i = 0;
        for (auto &item : itemVector) {
            if (item.front() == '{' && item.back() == '}') {
                keyPoint.push_back(i);
                keySet.push_back(item.substr(1, item.size() - 2));
            }
            i++;
        }
    }
}
bool RequestMapper::Key::MatchFilter(const std::string &reqPath, std::map<std::string, std::string> &valMap) {
    if (!needval)
        return reqPath == pattern;
    else {
        auto itemList = utils::split(reqPath, '/');
        auto patternList = utils::split(pattern, '/');
        // reqpath: /index/ 
        // pattern: /index/{user}/{pass}  false
        if(itemList.size() != patternList.size())
            return false;
        // reqpath: /index/x/y
        // pattern: /indep/{user}/{pass}  false
        for(int i = 0; i < keyPoint.front(); i++){
            if(itemList[i] != patternList[i])
                return false;
        }
        for (auto i = 0, j = 0; i < keyPoint.size(); i++) {
            int pos = keyPoint[i];
            if (pos >= itemList.size())
                break;
            valMap.insert(std::pair<std::string, std::string>(keySet[j++], itemList[pos]));
        }
        return true;
    }
}

bool RequestMapper::Key::MatchFilter(const std::string &reqPath) {
    return reqPath == pattern;
}

void RequestMapper::addRequestMapping(const Key &key, http::Func &&F) {
    m_vRequestMapper.push_back(std::pair<RequestMapper::Key, http::Func>(key, F));
}

http::Func RequestMapper::find(const std::string &RequestPath, std::map<std::string, std::string> &resultMap) {
    for (auto iter : m_vRequestMapper) {
        if (iter.first.MatchFilter(RequestPath, resultMap)){
            logger.info("request path:%s, handle path:%s", RequestPath, iter.first.pattern);
            return iter.second;
        }
    }
    logger.info("redirect 404.");
    return find(NOTFOUND, resultMap);
}
http::Func RequestMapper::find(const std::string &RequestPath) {
    for (auto iter : m_vRequestMapper) {
        if (iter.first.MatchFilter(RequestPath)){
            logger.info("request path:%s, handle path:%s", RequestPath, iter.first.pattern);
            return iter.second;
        }
    }
    return [=](HttpRequest &request, HttpResponse &response) {
        response.setStatusMessage(404, "HTTP/1.1", "404 not found");
        response.setBodyString(NOTFOUNDHTML);
        return true;
    };
}

ClientThread::ClientThread(int serverFd, int clientFd) : m_nThreadserverFd(serverFd), m_nThreadClientFd(clientFd) {}

void ClientThread::parseHeader(HttpRequest &request) {
    char temp[BUF_SIZE];
    bool bFindBody = false;
    std::string strLine, strKey, strVal;
    int LineCnt = 0;
    int BodySizeInRequest, recvSize = 0;
    Encoding EncodingType;
    int HeaderSize = 0;
    std::string HttpVersion, RequestType, RequestPath;
    MyStringBuffer mybuf;
    while (!bFindBody) {
        int nRead = recvData(m_nThreadClientFd, temp, BUF_SIZE, 0);
        if (nRead <= 0)
            return;
        for (size_t i = 0; i < nRead;) {
            if (i + 1 < nRead && temp[i] == '\r' && temp[i + 1] == '\n' && !bFindBody) {
                if (LineCnt == 0) {
                    ParseFirstLine(strLine, HttpVersion, RequestPath, RequestType);
                    request.setRequestType(RequestType);
                    request.setRequestPath(RequestPath);
                    request.setHttpVersion(HttpVersion);
                    // logger.info("httpversion:%s requesttype:%s requestpath:%s", HttpVersion, RequestType, RequestPath);
                    LineCnt++;
                } else if (!strLine.empty()) {
                    ParseHeaderLine(strLine, strKey, strVal);
                    strKey = utils::trim(strKey);
                    strVal = utils::trim(strVal);
                    // logger.debug("Find Response Body Header: %s->%s", strKey, strVal);
                    if (strcasecmp(strKey.c_str(), ContentLength) == 0) {
                        BodySizeInRequest = atoi(strVal.c_str());
                        EncodingType = EncodingLength;
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
            } else if (!bFindBody) {  // Header
                strLine.push_back(temp[i]);
                i++;
            } else if (strcasecmp(RequestType.c_str(), "get") == 0) {
                break;
            } else if (strcasecmp(RequestType.c_str(), "post") == 0 && BodySizeInRequest != 0) {
                mybuf.sputc(temp[i]);
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
        if (line[i] == ':')
            if (nBlankCnt == 0)
                nBlankCnt = 1;
            else
                val.push_back(line[i]);
        else if (nBlankCnt == 0) {
            key.push_back(line[i]);
        } else if (nBlankCnt == 1) {
            val.push_back(line[i]);
        }
    }
}

void ClientThread::ParseFirstLine(const std::string &line, std::string &HttpVersion, std::string &RequestPath, std::string &RequestType) {
    int nBlankCnt = 0;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == ' ') {
            if (nBlankCnt != 2)
                nBlankCnt++;
            else
                HttpVersion.push_back(line[i]);
        } else if (nBlankCnt == 0) {
            RequestType.push_back(line[i]);
        } else if (nBlankCnt == 1) {
            RequestPath.push_back(line[i]);
        } else if (nBlankCnt == 2) {
            HttpVersion.push_back(line[i]);
        }
    }
}
void ClientThread::handRequest(RequestMapper *handerMapping, HttpConfig *config, const char *strRemoteIP) {
    HttpRequest request;
    HttpResponse response;
    parseHeader(request);
    std::map<std::string, std::string> recvHeaderMap;
    logger.info("status: %d", utils::CheckImageSuffix(request.getRequestPath()));
    int nWrite;
    if(!utils::CheckImageSuffix(request.getRequestPath())){
        http::Func iter = handerMapping->find(request.getRequestPath(), recvHeaderMap);
        iter(request, response);
        nWrite=writeResponse(response);
    }else{
        // resource
        http::Func iter = handerMapping->find("/#/");
        iter(request, response);
        nWrite=response.WriteBytes(m_nThreadClientFd);
    }
    logger.info("%s -- [%s] \"%s %s %s\" %d %d \"-\" \"%s\" \"-\"", strRemoteIP, utils::requstTimeFmt(), request.getRequestType(), request.getRequestPath(), request.getHttpVersion(),
    response.getStatusCode(), nWrite, request.get(UserAgent));
    logger.info("response has been sent.");
}
ssize_t ClientThread::writeResponse(HttpResponse &response) {
    std::string responseHeader = response.toResponseHeader();
    return writeData(m_nThreadClientFd, (void *)responseHeader.c_str(), responseHeader.size(), 0);
}

int ClientThread::recvData(int fd, void *buf, size_t n, int ops) {
    return ::recv(fd, buf, n, ops);
}
int ClientThread::writeData(int fd, void *buf, size_t n, int ops) {
    return ::send(fd, buf, n, ops);
}

HttpServer::HttpServer(const std::string &strServerName, const std::string &strServerIP, const std::string &strServerDescription, int nPort) :
        m_strServerName(strServerName), m_strServerIP(strServerIP), m_strServerDescription(strServerDescription), m_nPort(nPort), m_nMaxListenClients(20), m_nServerFd(-1), m_nEpollTimeOut(10) {}

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
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(m_nPort);
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
    event.events = EPOLLIN;
    event.data.fd = m_nServerFd;  //服务器绑定的fd
    struct epoll_event *event_client_array = new epoll_event[m_nMaxListenClients];
    // 3.注册事件,将当前的epoll_fd注册到内核中去用于事件的监听
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, m_nServerFd, &event);
    // 4. 进入事件等待
    while (true) {
        //设置最大的监听的数量,以及EPOLL 的超时时间
        int ready_count = epoll_wait(epoll_fd, event_client_array, m_nMaxListenClients, m_nEpollTimeOut);
        // 5.从就绪的event里面进行accept,用于创建链接到客户端
        for (int i = 0; i < ready_count; i++) {
            socklen_t addr_len;
            struct sockaddr_in client_addr;
            int client_fd;
            if (event_client_array[i].data.fd == m_nServerFd) {  //确定为连接服务器的请求
                int client_fd = accept(m_nServerFd, (struct sockaddr *)&client_addr, &addr_len);
                // 6.把接收到client的fd 添加到epoll的监听事件中去
                event.events = EPOLLIN;
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, m_nServerFd, &event);
                logger.info("recv client:%d from address %s ", client_fd, inet_ntoa(client_addr.sin_addr));
                if (client_fd != -1) {
                    ClientThread clientThread(m_nServerFd, client_fd);
                    std::thread tempThread(&ClientThread::handRequest, clientThread, &m_mapper, &m_mConfig, inet_ntoa(client_addr.sin_addr));
                    tempThread.join();
                } else {
                    logger.info("client fd is -1");
                }
            } else if (event_client_array[i].events & EPOLLIN) {
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

bool HttpServer::loadHttpConfig(const std::string &strHttpServerConfig) {
    bool ret = m_mConfig.loadConfig(strHttpServerConfig);
    return ret;
}

}  // namespace http