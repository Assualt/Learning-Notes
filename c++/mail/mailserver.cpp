#include "mailserver.h"
#include "base64.h"
#include "hashutils.hpp"
#include "log.h"
#include "mailcontext.h"
#include "mailutils.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
namespace mail {

MailServer::MailServer()
    : m_nMailServerSocket(-1) {
}
MailServer::~MailServer() {
    if (m_nMailServerSocket != -1)
        close(m_nMailServerSocket);
    if (m_ptrConfigMgr)
        delete m_ptrConfigMgr;
    if (m_ptrServerLogger)
        delete m_ptrServerLogger;
    if (m_ptrServerCommandLogger)
        delete m_ptrServerCommandLogger;
    if (m_ptrServerTransLogger)
        delete m_ptrServerTransLogger;
}

void MailServer::initEx(const std::string &strConfigPath) {

    m_ptrServerLogger        = new tlog::Logger(SERVER_LOGGER);
    m_ptrServerCommandLogger = new tlog::Logger(SERVER_COMMAND_LOGGER);
    m_ptrServerTransLogger   = new tlog::Logger(SERVER_TRANS_LOGGER);
    m_ptrServerLogger->BasicConfig("%(process)s %(threadname)s %(levelname)s %(ctime)s [%(filename)s-%(lineno)s-%(funcName)s] %(message)s", "test.log");
    m_ptrServerCommandLogger->BasicConfig("%(process)s %(threadname)s %(levelname)s %(ctime)s [%(filename)s-%(lineno)s-%(funcName)s] %(message)s", "");
    m_ptrServerTransLogger->BasicConfig("%(process)s %(threadname)s %(levelname)s %(ctime)s [%(filename)s-%(lineno)s-%(funcName)s] %(message)s", "");
    tlog::logImpl::AppendLogger(SERVER_LOGGER, m_ptrServerLogger);
    tlog::logImpl::AppendLogger(SERVER_COMMAND_LOGGER, m_ptrServerCommandLogger);
    tlog::logImpl::AppendLogger(SERVER_TRANS_LOGGER, m_ptrServerTransLogger);
    m_ptrConfigMgr = new conf::ConfigureManager(strConfigPath);
    m_ptrConfigMgr->init();
    MailEnv::getInstance().initMailEnv(*m_ptrConfigMgr);
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
        StartCommandEvent(nThreadCount);
        logger.info("run in father process, wait child process exit...");
        waitpid(pid, &status, 0);
        logger.info("child process exit. and status is %d", WEXITSTATUS(status));
    }
}

bool MailServer::StartCommandEvent(size_t nThreadCount) {
    m_MailCommandThreadsPool.startPool(nThreadCount);
    auto pEnv = &MailEnv::getInstance();
    int  ret  = 0;
    // 设置 每个进程允许打开的最大文件数
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = 1000;
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1) {
        logger.warning("setrlimit failed. %s", strerror(errno));
        return false;
    }
    int nCommandListenSocket = -1;
    // 1.创建server socket 用作监听使用，使用TCP可靠的连接和 IPV4的形式进行事件进行监听，协议使用IPV4
    nCommandListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (nCommandListenSocket == -1) {
        logger.warning("create socket error with erros:%s", strerror(errno));
        return false;
    }

    int yes = 1;
    if (setsockopt(nCommandListenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        logger.warning("set address reuse failed");
        return false;
    }
    // 设置端口为非阻塞模式
    // int oldFlag = setFDnonBlock(m_nServerFd);
    // 2.server端进行bind 并初始化server的地址结构体, 注意字节序的转化
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(pEnv->getServerIP().c_str());
    server_addr.sin_port        = htons(pEnv->getCommandPort());
    if ((ret = bind(nCommandListenSocket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) == -1) {
        logger.warning("bind port %d error with erros:%s", pEnv->getCommandPort(), strerror(errno));
        return false;
    }
    // 3.server端进行监听
    if ((ret = listen(nCommandListenSocket, pEnv->getMaxClient())) == -1) {
        logger.warning("listen error with erros:%s", strerror(errno));
        return false;
    }

    logger.info("MailServer command to listen at %s:%d ...", pEnv->getServerIP(), pEnv->getCommandPort());

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t          addr_len  = sizeof(client_addr);
        int                client_fd = accept(nCommandListenSocket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd != -1) {
            if (m_MailCommandThreadsPool.idlCount() < 1) {
                logger.info("current thread pool is full . closing transmission channel.");
                close(client_fd);
            } else {
                logger.info("begin to assign one thread to handle such request from client_fd:%d , thread pool left size:%d", client_fd, m_MailQueueThreadsPool.idlCount());
                ConnectionInfo info;
                info.m_strConnectIP = inet_ntoa(client_addr.sin_addr);
                info.m_nPort        = ntohs(client_addr.sin_port);
                info.m_nClientFd    = client_fd;
                info.m_nFDFlag      = 10;
                m_MailCommandThreadsPool.commit(MailServer::HandleCommandRequest, &info);
            }
        }
    }
    close(nCommandListenSocket);
}

bool MailServer::WaitEvent(size_t nThreadCount) {
    m_MailQueueThreadsPool.startPool(nThreadCount);
    m_MailCommandThreadsPool.startPool(nThreadCount);
    auto pEnv = &MailEnv::getInstance();
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
    int yes = 1;
    if (setsockopt(m_nMailServerSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        logger.warning("set address reuse failed");
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
        logger.warning("bind port %d error with erros:%s", pEnv->getServerPort(), strerror(errno));
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
MailProcess::MailProcess(MailContext &MailContext)
    : m_MailContext(MailContext) {
}

void MailServer::HandleCommandRequest(ConnectionInfo *info) {
    char buf[ 1024 ];
    while (1) {
        memset(buf, 0, 1024);
        size_t nRead = ::recv(info->m_nClientFd, buf, 1024, 0);
        if (nRead < 0) {
            break;
        }
        std::string command = Utils::trimRight(std::string(buf, nRead), std::string("\r\n"));
        if (strncasecmp(command.c_str(), "quit", 4) == 0) {
            commandlogger.info("recv app command quit.");
            break;
        } else if (strncasecmp(command.c_str(), "help", 4) == 0) {
            size_t nWrite = ::send(info->m_nClientFd, SERVER_APP_HELP_MESSGAE, strlen(SERVER_APP_HELP_MESSGAE), 0);
            commandlogger.debug("write App command bytes %d", nWrite);
        } else if (strncasecmp(command.c_str(), "version", 7) == 0) {
            std::string BuildRelease = MailEnv::getInstance().getBuildVersionDate();
            BuildRelease.append("\r\n");
            size_t nWrite = ::send(info->m_nClientFd, (void *)BuildRelease.c_str(), BuildRelease.size(), 0);
            commandlogger.debug("write App command bytes %d", nWrite);
        } else {
            size_t nWrite = ::send(info->m_nClientFd, "UnSupported Command.\r\n", 22, 0);
            commandlogger.debug("write App command bytes %d", nWrite);
        }
    }
    close(info->m_nClientFd);
}

void MailServer::HandleRequest(ConnectionInfo *info) {

    char        buf[ MAX_BUF_SIZE ];
    MailContext context;
    MailProcess handleProcess(context);
    int         old_flag = fcntl(info->m_nClientFd, F_GETFD, 0);
    fcntl(info->m_nClientFd, F_SETFD, old_flag | O_NONBLOCK);
    MAIL_STATE        state = HELO;
    std::stringstream ss;

    std::string strWelComeMessage = Welcome_Message(MailEnv::getInstance().getPrimaryDomain(), MailEnv::getInstance().getBuildVersionDate());
    int         nWrite            = ::send(info->m_nClientFd, (void *)strWelComeMessage.c_str(), strWelComeMessage.size(), 0);
    if (nWrite != strWelComeMessage.size()) {
        serverlogger.warning("write data to client %d failed. write size:%d left size:%d", info->m_nClientFd, nWrite, strWelComeMessage.size() - nWrite);
        close(info->m_nClientFd);
        return;
    }

    while (1) {
        memset(buf, 0, MAX_BUF_SIZE);
        size_t nRead = ::recv(info->m_nClientFd, buf, MAX_BUF_SIZE, 0);
        if (nRead < 0) {
            if (errno == EAGAIN) {
                serverlogger.info("eagain");
                continue;
            } else {
                serverlogger.warning("recv from client fd:%d error. %s\\r\\n", info->m_nClientFd, strerror(errno));
                break;
            }
        } else if (nRead == 0) {

            continue;
        } else {
            serverlogger.info("recv body buffer from client fd:%d, %s state:%d", info->m_nClientFd, std::string(buf, nRead - 2), state);
            if (strncasecmp(buf, "quit", 4) == 0) {
                logger.info("recv quit command . and disconnect now..");
                ::send(info->m_nClientFd, "221 bye\r\n", 9, 0);
                fcntl(info->m_nClientFd, F_SETFD, old_flag);
                break;
            } else if (state == DATAFINISH) {
                std::string strReplyString;
                std::string tempBuf(buf, nRead);
                ss << tempBuf;
                if (ss.str().find("\r\n.\r\n") == ss.str().size() - 5) { // recv finished.
                    serverlogger.info("recv finished.");
                    state         = handleProcess.process(state, ss.str().substr(0, ss.str().size() - 5), strReplyString);
                    size_t nWrite = ::send(info->m_nClientFd, strReplyString.c_str(), strReplyString.size(), 0);
                    serverlogger.info("write buffer size:%d ReplayString:%s", nWrite, strReplyString.substr(0, strReplyString.size() - 2));
                }
            } else if (state == DISCONNECT) {
                serverlogger.info("421 close transimission channel");
                ::send(info->m_nClientFd, "221 bye\r\n", 9, 0);
                break;
            } else {
                if (strcasecmp(buf, "data\r\n") == 0) {
                    state = DATA;
                }
                std::string strReplyString;
                state         = handleProcess.process(state, std::string(buf, nRead), strReplyString);
                size_t nWrite = ::send(info->m_nClientFd, strReplyString.c_str(), strReplyString.size(), 0);
                serverlogger.info("write buffer size:%d ReplayString:%s", nWrite, strReplyString.substr(0, strReplyString.size() - 2));
            }
        }
    }
    close(info->m_nClientFd);
}

TIgnoreCaseSet MailProcess::m_SupportMethods{"helo", "ehlo", "starttls", "mail from", "rcpt to", "data", "vrfy", "expn", "help", "rset", "quit", "auth login"};

bool MailProcess::SupportCommand(const std::string &strMethod) {
    return m_SupportMethods.count(strMethod);
}
bool MailProcess::isValidMailBox(const std::string &strMailBox) {
    size_t nPos   = strMailBox.find("@");
    size_t nCount = Utils::count(strMailBox, '@');
    return nPos != std::string::npos && nPos != 0 && nPos != strMailBox.size() && nCount == 1;
}
std::pair<std::string, std::string> MailProcess::getCommandVal(const std::string &strCmd) {
    std::string strKey, strVal;
    bool        bFindKey     = false;
    std::string RemoveString = Utils::trimRight(strCmd, std::string("\r\n"));
    for (size_t i = 0; i < RemoveString.size(); ++i) {
        if (!bFindKey && SupportCommand(strKey)) {
            bFindKey = true;
        } else if (!bFindKey) {
            strKey.push_back(tolower(RemoveString[ i ]));
        } else if (bFindKey) {
            if ((i + 1) < RemoveString.size() && RemoveString[ i ] == '\r' && RemoveString[ i + 1 ] == '\n')
                break;
            else if (!isblank(RemoveString[ i ]) && RemoveString[ i ] != ':')
                strVal.push_back(RemoveString[ i ]);
        }
    }
    if (bFindKey)
        return std::pair<std::string, std::string>(strKey, strVal);
    return std::pair<std::string, std::string>("", "");
}

std::pair<std::string, std::string> MailProcess::SplitMailAddress(const std::string &strCmd) {
    bool        bFindAt = false;
    std::string strKey, strVal;
    for (size_t i = 0; i < strCmd.size(); ++i) {
        if (strCmd[ i ] == '@')
            bFindAt = true;
        else if (!bFindAt)
            strKey.push_back(strCmd[ i ]);
        else
            strVal.push_back(strCmd[ i ]);
    }
    if (bFindAt)
        return std::pair<std::string, std::string>(strKey, strVal);
    return std::pair<std::string, std::string>("", "");
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
        case REST:
            return onRest(BufString, ReplyString);
    }
    return DISCONNECT;
}

MAIL_STATE MailProcess::onHELO(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "ehlo ", 5) == 0)
        return onEHLO(BufString, ReplyString);
    else if (strncasecmp(BufString.c_str(), "helo ", 5) != 0) {
        ReplyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    }
    m_MailContext.m_strEhloDomain = BufString.substr(5, BufString.find("\r\n") - 5);
    ReplyString.assign("250 OK\r\n");
    return AUTH;
}
MAIL_STATE MailProcess::onEHLO(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "ehlo ", 5) != 0) {
        ReplyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    }
    m_MailContext.m_strEhloDomain = BufString.substr(5, BufString.find("\r\n") - 5);
    ReplyString.assign("250-mail\r\n");
    ReplyString.append("250-PIPELINING\r\n");
    ReplyString.append("250-AUTH LOGIN\r\n");
    ReplyString.append("250-AUTH=LOGIN\r\n");
    ReplyString.append("250-STARTTLS\r\n");
    ReplyString.append("250-SMTPUTF8\r\n");
    ReplyString.append("250 8BITMIME\r\n");
    return AUTH;
}
MAIL_STATE MailProcess::onAuth(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "mail from", 9) == 0) {
        return onMailFrom(BufString, ReplyString);
    } else if (strcasecmp(BufString.c_str(), "auth login\r\n") == 0) { // 250-AUTH login\r\n
        ReplyString.assign("354 dXNlcm5hbWU6\r\n");
        return AUTHPASS;
    } else if (strncasecmp(BufString.c_str(), "auth plain", 10) == 0) { // 250-AUTH PLAIN xxx:xxx\r\n
        return onAuthPLAIN(BufString, ReplyString);
    } else if (strncasecmp(BufString.c_str(), "auth login", 10) == 0) { // 250-AUTH LOGIN xxx
        std::string tempString = Utils::trim(BufString.substr(10), std::string("\r\n"));
        return onAuthPass(tempString, ReplyString);
    }
    ReplyString.assign(SERVER_Response_UnSupportCommand);
    return DISCONNECT;
}

MAIL_STATE MailProcess::onAuthPass(const std::string &BufString, std::string &ReplyString) {
    ReplyString.assign("334 UGFzc3dvcmQ6\r\n");
    std::string strTempAuthUser = Utils::trimRight(BufString, std::string("\r\n"));
    std::string Result;
    if (HashUtils::DecodeBase64(strTempAuthUser, Result) <= 0 || Result.empty() || !isValidMailBox(Result)) {
        logger.info("user is not base64 encode. invalid input authuser %s.", strTempAuthUser);
        ReplyString.assign("550 Auth Failed.\r\n");
        return DISCONNECT;
    }
    // check auth user is exists or not
    m_MailContext.m_strAuthUser = Result;
    logger.info("auth login user:%s", Result);
    return AUTHEND;
}

MAIL_STATE MailProcess::onAuthEND(const std::string &BufString, std::string &ReplyString) {
    std::string strTempAuthPass = BufString.substr(0, BufString.size() - 2);
    std::string Result;
    if (HashUtils::DecodeBase64(strTempAuthPass, Result) <= 0 || Result.empty()) {
        logger.info("pass is not base64 encode. invalid input Pass %s.", strTempAuthPass);
        ReplyString.assign("550 Auth User/Pass is invalid\r\n");
        return DISCONNECT;
    }
    // 这里查询后端接口 用于校验是否Auth OK.
    m_MailContext.m_strAuthPass = Result;
    ReplyString.assign("235 Authentication successful\r\n");
    logger.info("get auth user:%s pass:%s", m_MailContext.m_strAuthUser, m_MailContext.m_strAuthPass);
    m_bAuthPassed = true;
    return MAILFROM;
}

MAIL_STATE MailProcess::onAuthPLAIN(const std::string &BufString, std::string &ReplyString) {
    std::string strCommand = Utils::trimRight(BufString, std::string("\r\n"));
    auto        UserPass   = Utils::split(strCommand, ' ');
    if (UserPass.size() != 3) {
        ReplyString.assign("502 Error.Syntax error.\r\n");
        return DISCONNECT;
    }
    std::string DecodeUserPass;
    int         nSize        = HashUtils::DecodeBase64(UserPass[ 2 ], DecodeUserPass);
    auto        tempUserPass = Utils::split(DecodeUserPass, '\0');
    if (tempUserPass.size() < 2) {
        ReplyString.assign("502 Error.Syntax error.\r\n");
        return DISCONNECT;
    }
    m_MailContext.m_strAuthUser = tempUserPass[ tempUserPass.size() - 2 ];
    m_MailContext.m_strAuthPass = tempUserPass.back();
    logger.debug("decode %s byte %d --> auth user:%s auth pass:%s", BufString, nSize, m_MailContext.m_strAuthUser, m_MailContext.m_strAuthPass);
    ReplyString.assign("235 Authentication successful\r\n");
    m_bAuthPassed = true;
    return MAILFROM;
}

MAIL_STATE MailProcess::onMailFrom(const std::string &BufString, std::string &ReplyString) {
    if (strncasecmp(BufString.c_str(), "mail from:", 10) != 0) {
        ReplyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    }
    auto mailPair = getCommandVal(BufString);
    if (!SupportCommand(mailPair.first)) {
        ReplyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    } else if (strcasecmp(mailPair.first.c_str(), "mail from") != 0) {
        ReplyString.assign(SERVER_Response_BadSequence);
        return DISCONNECT;
    }

    if (mailPair.second.front() != '<' && mailPair.second.back() != '>') {
        logger.warning("invalid mail from:%s", mailPair.second);
        ReplyString.assign("550 invalid user" + mailPair.second + "\r\n");
        return DISCONNECT;
    }

    auto MailAddress = Utils::trimLeft(Utils::trimRight(mailPair.second, std::string(">")), std::string("<"));
    logger.info("mail from:%s", MailAddress);
    auto vec = SplitMailAddress(MailAddress);
    if (vec.first.empty() && vec.second.empty()) {
        logger.warning("invalid mail from: %s", MailAddress);
        ReplyString.assign("550 invalid user\r\n");
        return DISCONNECT;
    }
    if (MailEnv::getInstance().NeedAuth(vec.second) && !m_bAuthPassed) {
        ReplyString.assign("553 authentication is required\r\n");
        return DISCONNECT;
    }
    m_MailContext.m_strMailFrom = MailAddress;
    ReplyString.assign("250 Mail OK\r\n");
    return RCPTTO;
}
MAIL_STATE MailProcess::onRcptTo(const std::string &BufString, std::string &ReplyString) {
    auto command = getCommandVal(BufString);
    if (!SupportCommand(command.first)) {
        ReplyString.assign(SERVER_Response_UnSupportCommand);
        return DISCONNECT;
    } else if (strcasecmp(command.first.c_str(), "rcpt to") != 0) {
        ReplyString.assign(SERVER_Response_BadSequence);
        return DISCONNECT;
    }
    if (command.second.front() != '<' && command.second.back() != '>') {
        logger.warning("invalid mail from:%s", command.second);
        ReplyString.assign("550 invalid user" + command.second + "\r\n");
        return DISCONNECT;
    }
    auto MailAddress = Utils::trimLeft(Utils::trimRight(command.second, std::string(">")), std::string("<"));
    auto vec         = SplitMailAddress(MailAddress);
    if (vec.first.empty() && vec.second.empty()) {
        logger.warning("invalid mail from: %s", MailAddress);
        ReplyString.assign("550 invalid user\r\n");
        return DISCONNECT;
    }
    logger.info("rcpt to:%s", MailAddress);
    if (MailEnv::getInstance().NeedAuth(vec.second) && !m_bAuthPassed) {
        ReplyString.assign("553 authentication is required\r\n");
        return DISCONNECT;
    }
    // check rcpt is belong to such server

    // check rcpt whether equals to the mail from
    if (m_MailContext.m_strMailFrom == MailAddress)
        m_MailContext.m_RcptSets.insert(MailAddress);
    ReplyString.assign("250 Mail OK\r\n");

    return RCPTTO;
}
MAIL_STATE MailProcess::onData(const std::string &BufString, std::string &ReplyString) {
    if (strcasecmp(BufString.c_str(), "data\r\n") != 0) {
        ReplyString.assign(SERVER_Response_BadSequence);
        return DISCONNECT;
    }
    ReplyString.assign("354 End data with <CR><LF>.<CR><LF>\r\n");
    return DATAFINISH;
}

MAIL_STATE MailProcess::onDataFinish(const std::string &BufString, std::string &Replystring) {
    Replystring.assign("250 Mail OK queued as smtp with id \r\n");
    return DISCONNECT;
}

MAIL_STATE MailProcess::onRest(const std::string &BufString, std::string &ReplyString) {
    ReplyString.assign("250 OK\r\n");
    m_MailContext.clearContext();
    return AUTH;
}
} // namespace mail