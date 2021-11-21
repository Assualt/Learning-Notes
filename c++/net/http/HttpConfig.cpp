#include "HttpConfig.h"
#include "HttpUtils.h"
HttpConfig::HttpConfig(const char *configPath) {
    Init(configPath);
}

void HttpConfig::Init(const std::string &path) {
    setServerRoot("/home/xhou");

    m_DirentTmplateHtml = utils::loadFileString("/media/xhou/Data/git/Learning-Notes/c++/net/http/resource/dirHtml.tmpl");
}