#include "httpconfig.h"
namespace http {
HttpConfig::HttpConfig()
    : m_SuffixSet{"index.html", "index.shtml", "index.htm"} {
}

bool HttpConfig::loadConfig(const std::string &strConfigFilePath) {
    std::ifstream fin(strConfigFilePath.c_str());
    if (!fin.is_open()) {
        logger.error("load httconfig(%s) failed due to nonexistance.", strConfigFilePath);
        return false;
    }
    std::string strLine;
    while (getline(fin, strLine)) {
    }
    fin.close();
    return true;
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

const std::string HttpConfig::getMimeType(const std::string &strFileName) {
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
} // namespace http