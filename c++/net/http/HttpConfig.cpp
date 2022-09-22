#include "HttpConfig.h"
#include "HttpUtils.h"

using namespace muduo::base;

HttpConfig::HttpConfig(const char *configPath) { Init(configPath); }

void HttpConfig::Init(const std::string &path) {
    mgr_ = std::make_shared<ConfigureManager>(path, ".conf");
    mgr_->Init();

    mgr_->changeAccessPath("/httpd/server/http/");
    setServerRoot(mgr_->getString("/home/xhou", "ServerRoot"));
    setSupportWebSocket(mgr_->getBool(false, "SupportWebSocket"));
    setWebSocketThreadNum(mgr_->getInt(10, "WebSocketThreadNum"));
    setWebSocketPort(mgr_->getInt(3000, "WebSocketPort"));

    setDirentTmplateHtml(utils::loadFileString(
        mgr_->getString("/media/xhou/Data/git/Learning-Notes/c++/net/http/resource/dirHtml.tmpl", "DirentTmpl")));
}
