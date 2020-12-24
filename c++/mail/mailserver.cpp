#include "mailserver.h"
#include "base64.h"
#include "logging.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
namespace mail {

MailServer::MailServer()
    : m_nMailServerSocket(-1) {
}

void MailServer::initEx(const std::string &strConfigPath) {
    if (!m_ConfigMgr.initEx(strConfigPath)) {
        logger.info("init configuration env failed.");
        return;
    }
    MailEnv::getInstance().initMailEnv(m_ConfigMgr);
}

void MailServer::startServer(size_t nThreadCount) {
    int pid = fork();
    int status;
    if (pid == -1) {
        logger.warning("fork error. exiting ....");
        exit(1);
    } else if (pid == 0) { // child process.
        WaitEvent(nThreadCount);
    } else {
        logger.info("run in father process, wait child process exit...");
        waitpid(pid, &status, 0);
        logger.info("child process exit. and status is %d", WEXITSTATUS(status));
    }
}

bool MailServer::WaitEvent(size_t nThreadCount) {
    m_MailQueueThreadsPool.startPool(nThreadCount);
    auto pEnv = &MailEnv::getInstance();
    int  n    = 10;
    int  ret  = 0;
    // 设置 每个进程允许打开的最大文件数
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = 1000;
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1) {
        logger.warning("setrlimit failed. %s", strerror(errno));
        return false;
    }
    logger.info("set rlimit ok.");

    // 1.创建server socket 用作监听使用，使用TCP可靠的连接和 IPV4的形式进行事件进行监听，协议使用IPV4
    m_nMailServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_nMailServerSocket == -1) {
        logger.warning("create socket error with erros:%s", strerror(errno));
        return false;
    }
    // 设置端口为非阻塞模式
    // int oldFlag = setFDnonBlock(m_nServerFd);
    // 2.server端进行bind 并初始化server的地址结构体, 注意字节序的转化
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(pEnv->getServerIP().c_str());
    server_addr.sin_port        = htons(pEnv->getServerPort());
    if ((ret = bind(m_nMailServerSocket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1) {
        logger.warning("bind error with erros:%s", strerror(errno));
        return false;
    }
    // 3.server端进行监听
    if ((ret = listen(m_nMailServerSocket, pEnv->getMaxClient())) == -1) {
        logger.warning("listen error with erros:%s", strerror(errno));
        return false;
    }

    logger.info("MailServer begin to listen at %s:%d ...", pEnv->getServerIP(), pEnv->getServerPort());

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t          addr_len  = sizeof(client_addr);
        int                client_fd = accept(m_nMailServerSocket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd != -1) {
            if (m_MailQueueThreadsPool.idlCount() < 1) {
                logger.info("current thread pool is full . closing transmission channel.");
                close(client_fd);
            } else {
                logger.info("begin to assign one thread to handle such request from client_fd:%d , thread pool left size:%d", client_fd, m_MailQueueThreadsPool.idlCount());
                ConnectionInfo info;
                info.m_strConnectIP = inet_ntoa(client_addr.sin_addr);
                info.m_nPort        = ntohs(client_addr.sin_port);
                info.m_nClientFd    = client_fd;
                info.m_nFDFlag      = 10;
                m_MailQueueThreadsPool.commit(MailServer::HandleRequest, &info);
            }
        }
    }

    close(m_nMailServerSocket);
}

void MailServer::HandleRequest(ConnectionInfo *info) {
    char        buf[ MAX_BUF_SIZE ];
    MailProcess handleProcess;
    int         old_flag = fcntl(info->m_nClientFd, F_GETFD, 0);
    fcntl(info->m_nClientFd, F_SETFD, old_flag | O_NONBLOCK);
    MAIL_STATE        state = HELO;
    std::stringstream ss;
    while (1) {
        memset(buf, 0, MAX_BUF_SIZE);
        size_t nRead = ::recv(info->m_nClientFd, buf, MAX_BUF_SIZE, 0);
        if (nRead < 0) {
            if (errno == EAGAIN) {
                logger.info("eagain");
                continue;
            } else {
                logger.warning("recv from client fd:%d error. %s\\r\\n", info->m_nClientFd, strerror(errno));
                break;
            }
        } else if (nRead == 0) {
            logger.info("nRead is empty");
            continue;
        } else {
            logger.info("recv body buffer from client fd:%d, %s state:%d", info->m_nClientFd, std::string(buf, nRead - 2), state);
            if (strncasecmp(buf, "quit", 4) == 0) {
                logger.info("recv quit command . and disconnect now..");
                fcntl(info->m_nClientFd, F_SETFD, old_flag);
                break;
            } else if (state == DATAFINISH) {
                std::string strReplyString;
                std::string tempBuf(buf, nRead);
                ss << tempBuf;
                if (ss.str().find("\r\n.\r\n") == ss.str().size() - 5) { // recv finished.
                    logger.info("recv finished.");
                    state         = handleProcess.process(state, ss.str(), strReplyString);
                    size_t nWrite = ::send(info->m_nClientFd, strReplyString.c_str(), strReplyString.size(), 0);
                    logger.info("write buffer size:%d ReplayString:%s", nWrite, strReplyString.substr(0, strReplyString.size() - 2));
                }
            } else {
                if (strcasecmp(buf, "data\r\n") == 0) {
                    state = DATA;
                }
                std::string strReplyString;
                state         = handleProcess.process(state, std::string(buf, nRead), strReplyString);
                size_t nWrite = ::send(info->m_nClientFd, strReplyString.c_str(), strReplyString.size(), 0);
                logger.info("write buffer size:%d ReplayString:%s", nWrite, strReplyString.substr(0, strReplyString.size() - 2));
            }
            if (state == DISCONNECT) {
                logger.info("421 close transimission channel");
                break;
            }
        }
    }
    close(info->m_nClientFd);
}

MAIL_STATE MailProcess::process(MAIL_STATE state, const std::string &BufString, std::string &ReplyString) {
    switch (state) {
        case HELO:
            return onHELO(BufString, ReplyString);
        case AUTH:
            return onAuth(BufString, ReplyString);
        case AUTHPASS:
            return onAuthPass(BufString, ReplyString);
        case AUTHEND:
            return onAuthEND(BufString, ReplyString);
        case MAILFROM:
            return onMailFrom(BufString, ReplyString);
        case RCPTTO:
            return onRcptTo(BufString, ReplyString);
        case DATA:
            return onData(BufString, ReplyString);
        case DATAFINISH:
            return onDataFinish(BufString, ReplyString);
    }
    return DISCONNECT;
}

MAIL_STATE MailProcess::onHELO(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "ehlo ", 5) == 0)
        return onEHLO(BufString, ReplyString);
    else if (strncasecmp(BufString.c_str(), "helo ", 5) != 0) {
        ReplyString.assign("500 UnSupported command\r\n");
        return DISCONNECT;
    }
    ReplyString.assign("250 OK\r\n");
    return AUTH;
}
MAIL_STATE MailProcess::onEHLO(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "ehlo ", 5) != 0) {
        ReplyString.assign("500 UnSupported command\r\n");
        return DISCONNECT;
    }
    ReplyString.assign("250 OK\r\n");
    return AUTH;
}
MAIL_STATE MailProcess::onAuth(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "mail from:", 10) == 0)
        return onMailFrom(BufString, ReplyString);
    else if (strcasecmp(BufString.c_str(), "auth login\r\n") != 0) {
        ReplyString.assign("500 UnSupported command\r\n");
        return DISCONNECT;
    }
    ReplyString.assign("354 dXNlcm5hbWU6\r\n");
    return AUTHPASS;
}

MAIL_STATE MailProcess::onAuthPass(const std::string &BufString, std::string &ReplyString) {
    ReplyString.assign("334 UGFzc3dvcmQ6\r\n");
    m_strAuthUser = BufString.substr(0, m_strAuthUser.size() - 2);
    return AUTHEND;
}

MAIL_STATE MailProcess::onAuthEND(const std::string &BufString, std::string &ReplyString) {
    m_strAuthPass = BufString.substr(0, m_strAuthUser.size() - 2);
    ReplyString.assign("235 Authentication successful\r\n");
    logger.info("get auth user:%s pass:%s", m_strAuthUser, m_strAuthPass);
    m_bAuthPassed = true;
    return MAILFROM;
}

MAIL_STATE MailProcess::onMailFrom(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "mail from:", 10) != 0) {
        ReplyString.assign("500 UnSupported command\r\n");
        return DISCONNECT;
    }
    std::string strMailFrom;
    std::string strUser, strDomain;
    bool        bFindAt = false;
    for (size_t i = 11; i < BufString.size(); i++) {
        if (BufString.at(i) == '\r' && (i + 1 < BufString.size()) && BufString.at(i+1) == '\n')
            break;
        if (BufString.at(i) != ' ') {
            if (BufString.at(i) == '@')
                bFindAt = true;
            else if (!bFindAt)
                strUser.push_back(BufString[ i ]);
            else
                strDomain.push_back(BufString[ i ]);
            strMailFrom.push_back(BufString[ i ]);
        }
    }
    if (!bFindAt || strUser.empty() || strDomain.empty()) {
        logger.warning("invalid mail from: %s", strMailFrom);
        ReplyString.assign("550 invalid user\r\n");
        return DISCONNECT;
    }
    logger.info("mail from:%s auth passeed:%d", strDomain, m_bAuthPassed);
    if (MailEnv::getInstance().NeedAuth(strDomain) && !m_bAuthPassed) {
        ReplyString.assign("510 Authentication is Required\r\n");
        return DISCONNECT;
    }

    ReplyString.assign("250 Mail OK\r\n");
    return RCPTTO;
}
MAIL_STATE MailProcess::onRcptTo(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "rcpt to:", 8) != 0) {
        ReplyString.assign("500 UnSupported command\r\n");
        return DISCONNECT;
    }
    ReplyString.assign("250 Mail OK\r\n");
    return RCPTTO;
}
MAIL_STATE MailProcess::onData(const std::string &BufString, std::string &ReplyString) {
    if (strcasecmp(BufString.c_str(), "data\r\n") != 0) {
        ReplyString.assign("500 UnSupported command\r\n");
        return DISCONNECT;
    }
    ReplyString.assign("354 End data with <CR><LF>.<CR><LF>\r\n");
    return DATAFINISH;
}

MAIL_STATE MailProcess::onDataFinish(const std::string &BufString, std::string &Replystring) {
    Replystring.assign("250 Mail OK queued as smtp with id \r\n");
    return DISCONNECT;
}

} // namespace mail