#include "httpconfig.h"
namespace http {
HttpConfig::HttpConfig()
    : m_SuffixSet{"index.html", "index.shtml", "index.htm"}
    , m_bRequiredAuth(false) {
}
bool HttpConfig::needAuth() {
    return m_bRequiredAuth;
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
    std::string strLine;
    std::string strSection;
    size_t      nPos;
    bool        sectionStart = false;
    std::string strSectionName;
    while (getline(fin, strLine)) {
        strLine = utils::trim(strLine);
        if (strLine.empty())
            continue;
        else if (strLine[ 0 ] == '#')
            continue;
        else if ((nPos = strLine.find("{")) != std::string::npos && !sectionStart) {
            strSectionName = utils::trim(strLine.substr(0, nPos));
            sectionStart   = true;
        } else if ((nPos = strLine.find("}")) != std::string::npos && sectionStart) {
            sectionStart = false;
            // std::cout << "Name:" << strSectionName << " {" << strSection << "}" << std::endl;
            parseSection(strSectionName, strSection);
            strSectionName.clear();
            strSection.clear();
            sectionStart = false;
        } else if (sectionStart) {
            strSection += strLine;
        }
    }
    fin.close();
    return true;
}

void HttpConfig::parseSection(const std::string strSectionName, const std::string &strSection) {
    std::map<std::string, std::string> sectionMap;
    auto                               vectors = utils::split(strSection, ';');
    for (auto vec : vectors) {
        auto kval = utils::split(vec, ' ');
        if (kval.size() != 2)
            kval[ 0 ] = utils::trim(kval[ 0 ]);
        kval[ 1 ] = utils::trim(kval[ 1 ]);
        if (kval.size() == 2 && !kval[ 0 ].empty() && !kval[ 1 ].empty()) {
            sectionMap[ kval[ 0 ] ] = kval[ 1 ];
            logger.debug("key:%s--->val:%s", kval[ 0 ], kval[ 1 ]);
        } else if (kval.size() >= 2) {
            logger.info("line: %s", vec);
        } else
            logger.warning("invalid line: %s", vec);
        if (strSectionName == "http")
            if (kval[ 0 ] == "basic_auth")
                loadAuthFile(kval[ 1 ]);
            else if (kval[ 0 ] == "mime_type")
                loadMimeType(kval[ 1 ]);
            else if (kval[ 0 ] == "dirent_tmpl")
                loadDirentTmplateHtml(kval[ 1 ]);
    }
    m_SectionMap[ strSectionName ] = sectionMap;
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

std::string HttpConfig::getServerRoot() const {
    return m_strServerRoot;
}
void HttpConfig::setServerRoot(const std::string &strServerRoot) {
    m_strServerRoot = strServerRoot;
}

void HttpConfig::loadDirentTmplateHtml(const std::string &tmplatePath) {
    m_strDirentTmplateHtml = utils::loadFileString(tmplatePath);
}
std::string &HttpConfig::getDirentTmplateHtml() {
    return m_strDirentTmplateHtml;
}

const std::set<std::string> HttpConfig::getSuffixSet() {
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
    }
    fin.close();
    logger.debug("success insert %d user into auth map", m_AuthPassMap.size());
    m_bRequiredAuth = true;
}
} // namespace http