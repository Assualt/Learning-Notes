#include "conf/xxmlconfmgr.h"

NAMESPACE_BEGIN

bool TXmlConfigureManager::init() {
    return initIniFiles();
}

bool TXmlConfigureManager::initIniFiles() {
    std::vector<tstring> fileNames = TConfigureManagerBase::getAllConfFiles();
    for (auto it : fileNames) {
        TXmlNode tempNode;
        initSingerFile(tempNode, it);
        if (m_mKVMap.find(it) == m_mKVMap.end()) {
            // it-> /a/b/c/d/e/f/x.cf
            // base->/a/b/c/d/e/f/
            // key->x
            tstring strKey;
            if (it.find(m_strConfigureDir) != tstring::npos) {
                strKey = it.substr(it.find(m_strConfigureDir) + m_strConfigureDir.size());
            }
            // remove suffix
            strKey = strKey.substr(1, strKey.rfind(getConfType()) - 1);
            m_mKVMap[strKey] = tempNode;
        }
    }

    return true;
}
void TXmlConfigureManager::initSingerFile(TXmlNode &vNode, const tstring &confPath) {
    



}

NAMESPACE_END