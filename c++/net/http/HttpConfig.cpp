#include "HttpConfig.h"
#include "HttpUtils.h"
#include "base/Logging.h"
#include "base/System.h"
using namespace muduo::base;

HttpConfig::HttpConfig(const char *configPath) { Init(configPath); }

void HttpConfig::Init(const std::string &path) {
    mgr_ = std::make_shared<ConfigureManager>(path, ".conf");
    mgr_->Init();

    mgr_->changeAccessPath("/httpd/server/http/");
    setServerRoot(mgr_->getString("/home/xhou", "ServerRoot"));
    setServerPort(mgr_->getInt(8000, "ServerPort"));
    setSupportSSL(mgr_->getBool(true, "SupportSSL"));
    setSupportWebSocket(mgr_->getBool(false, "SupportWebSocket"));
    setWebSocketThreadNum(mgr_->getInt(10, "WebSocketThreadNum"));
    setWebSocketPort(mgr_->getInt(3000, "WebSocketPort"));
    setDirentTmplateHtml(utils::loadFileString(mgr_->getString("", "DirentTmpl")));
    setSupportSSL(mgr_->getBool(false, "SupportSSL"));
    setSSLCertPath(mgr_->getString("", "SSLCertPath"));
    setSSLPrivateKeyPath(mgr_->getString("", "SSLPrivateKeyPath"));
    logger.info("init httpserver port:%d SSL:%b Ws:%b ssl:[%b:%s:%s] pwd:%s", m_ServerPort, m_SupportSSL,
                m_SupportWebSocket, m_SupportSSL, m_SSLCertPath, m_SSLPrivateKeyPath, System::CurrentPwd());
}
