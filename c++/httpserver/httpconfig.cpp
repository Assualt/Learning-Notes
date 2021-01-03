#include "httpconfig.h"
namespace http {
HttpConfig::HttpConfig()
    : m_bSSLServer(false)
    , m_SuffixSet{"index.html", "index.shtml", "index.htm"}
    , m_bRequiredAuth(false)
    , m_SupportMethodSet{"GET", "POST", "PUT", "DELETE", "HEAD"}
    , m_SupportHttpVersionSet{"Http/1.1", "Http/1.0"}
    , m_nMaxAcceptClients(20) {
}
HttpConfig::~HttpConfig() {
    if (m_ptrConfigure)
        delete m_ptrConfigure;
}
bool HttpConfig::needAuth() {
    return m_bRequiredAuth;
}

bool HttpConfig::checkMethod(const std::string &RequestMethod) {
    return m_SupportMethodSet.count(RequestMethod);
}
bool HttpConfig::checkHttpVersion(const std::string &HttpVersion) {
    return m_SupportHttpVersionSet.count(HttpVersion);
}

bool HttpConfig::checkAuth(const std::string &AuthString) {
    auto vecs = utils::split(AuthString, ' ');
    if (vecs.size() == 2) {
        if (strcasecmp(vecs[ 0 ].c_str(), "basic") == 0) {
            std::string decodeString;
            int         nDecode = HashUtils::DecodeBase64(vecs[ 1 ], decodeString);
            if (nDecode == -1) {
                logger.info("decode user auth:%s failed", vecs[ 1 ]);
                return false;
            }
            auto userpass = utils::split(decodeString, ':');
            if (!m_AuthPassMap.count(userpass[ 0 ])) {
                logger.info("such user %s is not accept", userpass[ 0 ]);
                return false;
            } else if (m_AuthPassMap[ userpass[ 0 ] ] != userpass[ 1 ]) {
                logger.info("such user %s password is error, input pass:%s, require pass:%s", userpass[ 0 ], m_AuthPassMap[ userpass[ 0 ] ], userpass[ 1 ]);
                return false;
            }
            return true;
        }
    }
    return false;
}
bool HttpConfig::loadConfig(const std::string &strConfigFilePath) {
    std::ifstream fin(strConfigFilePath.c_str());
    if (!fin.is_open()) {
        logger.error("load httconfig(%s) failed due to nonexistance.", strConfigFilePath);
        return false;
    }
    m_ptrConfigure = new conf::ConfigureManager(strConfigFilePath, ".conf");
    m_ptrConfigure->init();
    initSetting();
    return true;
}

void HttpConfig::initSetting() {
    std::string Prefix = "/httpd/";
    m_ptrConfigure->changeAccessPath(Prefix + "server/http/");
    m_strServerRoot        = m_ptrConfigure->getString("", "root");
    m_strDirentTmplateHtml = m_ptrConfigure->getString("", "DirentTmpl");
    loadDirentTmplateHtml(m_strDirentTmplateHtml);
    int AuthType = m_ptrConfigure->getInt(0, "AuthType");
    if (AuthType == 1) { // Auth Basic
        m_strAuthName             = m_ptrConfigure->getString("", "AuthBasic");
        std::string BasicAuthFile = m_ptrConfigure->getString("", "BasicAuthFile");
        loadAuthFile(BasicAuthFile);
        m_strAuthName = "Basic realm=\"" + m_strAuthName + "\"";
    }

    m_strLoggerFmt           = m_ptrConfigure->getString("", "log_format");
    std::string MimeTypeFile = m_ptrConfigure->getString("", "MimeTypeFile");
    m_nServerPort            = m_ptrConfigure->getInt(8080, "ListenPort");
    m_strServerHost          = m_ptrConfigure->getString("127.0.0.1", "Host");
    m_nMaxAcceptClients      = m_ptrConfigure->getInt(20, "MaxClients");

    // init ssl config
    m_ptrConfigure->changeAccessPath(Prefix + "server/ssl/");
    bool enableSSL = m_ptrConfigure->getBool(false, "EnableSSL");
    if (enableSSL) {
        m_sslconfig.certificate = m_ptrConfigure->getString("", "ssl_certificate");
        std::string protocol    = m_ptrConfigure->getString("", "ssl_protocols");
        if (strcasecmp(protocol.c_str(), "tlsv1.1") == 0)
            m_sslconfig.protocol = Type_TLS1_1;
        else if (strcasecmp(protocol.c_str(), "tlsv1.2") == 0)
            m_sslconfig.protocol = Type_TLS1_2;
        else if (strcasecmp(protocol.c_str(), "tlsv1.3") == 0)
            m_sslconfig.protocol = Type_TLS1_3;
        else {
            m_sslconfig.protocol = Type_TLS1_23;
            logger.warning("unsupported ssl encrypt version %s using default protocol 23", protocol);
        }
        m_sslconfig.certificate_private_key = m_ptrConfigure->getString("", "ssl_certificate_key");
        m_sslconfig.ciphers                 = m_ptrConfigure->getString("", "ssl_ciphers");
        m_sslconfig.ca                      = m_ptrConfigure->getString("", "ca_cert");
        m_bSSLServer                        = true;
    }

    loadMimeType(MimeTypeFile);
}

bool HttpConfig::loadMimeType(const std::string &mimeType) {
    std::ifstream fin(mimeType);
    if (!fin.is_open()) {
        logger.info("error load mime type from file %s", mimeType);
        return false;
    }
    std::string strLine;
    int         success = 0;
    while (getline(fin, strLine)) {
        if (strLine.find(";") != std::string::npos) {
            strLine = strLine.substr(0, strLine.find(";"));
        }
        if (strLine.empty())
            continue;
        auto tempList = utils::split(strLine, ' ');
        if (tempList.size() < 2)
            continue;
        for (size_t i = 1; i < tempList.size(); ++i) {
            m_ExtMimeType[ tempList[ i ] ] = tempList[ 0 ];
            success++;
        }
    }
    fin.close();
    logger.info("success insert %d mime<->ext mapping", success);
    return true;
}

std::string HttpConfig::getMimeType(const std::string &strFileName) {
    std::string ext;
    if (strFileName.rfind(".") != std::string::npos) {
        ext = strFileName.substr(strFileName.rfind(".") + 1);
        if (m_ExtMimeType.count(ext))
            return m_ExtMimeType.at(ext);
    }
    return utils::FileMagicType(strFileName);
}

void HttpConfig::loadDirentTmplateHtml(const std::string &tmplatePath) {
    m_strDirentTmplateHtml = utils::loadFileString(tmplatePath);
}
std::string &HttpConfig::getDirentTmplateHtml() {
    return m_strDirentTmplateHtml;
}

std::set<std::string, StringCaseCmp> HttpConfig::getSuffixSet() {
    return m_SuffixSet;
}

void HttpConfig::loadAuthFile(const std::string &strAuthFile) {
    std::ifstream fin(strAuthFile.c_str());
    if (!fin.is_open()) {
        logger.warning("open %s auth file failed.", strAuthFile);
        m_bRequiredAuth = false;
        return;
    }
    std::string line;
    while (getline(fin, line)) {
        line = utils::trim(line);
        if (strncasecmp(line.c_str(), "#", 1) == 0)
            continue;
        auto vec = utils::split(line, ':');
        if (vec.size() != 2)
            continue;
        m_AuthPassMap.insert(std::pair<std::string, std::string>(vec[ 0 ], vec[ 1 ]));
        logger.debug("insert user:[%s] -> pass:[%s]", vec[ 0 ], vec[ 1 ]);
    }
    fin.close();
    logger.debug("success insert %d user into auth map", m_AuthPassMap.size());
    m_bRequiredAuth = true;
}
} // namespace http