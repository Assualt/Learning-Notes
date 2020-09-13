#ifndef _X_XMLCONFMGR_H_2020_04_28
#define _X_XMLCONFMGR_H_2020_04_28

#include "tconfmgrbase.h"
#include "base/texception.h"
#include <map>
#include <vector>
NAMESPACE_BEGIN

class TXmlConfigureManager : public TConfigureManagerBase {
    // <tag attr1="v1" attr2="v2"> value </tag>
    template <class chr>
    struct xmlnode {
        typedef std::basic_string<chr> typeChar;

    protected:
        typeChar strTagName;   //  tag
        typeChar strTagValue;  //  Text
        std::map<typeChar, typeChar> mAttributeMap;
        struct xmlnode *ptr_Parent;
        std::map<typeChar, struct xmlnode *> mChildList;

    public:
        int getChildrenSize() const {
            return mChildList.size();
        }
        xmlnode *getChildByName(const typeChar name) const {
            if (mChildList.find(name) == mChildList.end())
                return NULL;
            return mChildList[name];
        }
        typeChar getTagName(void) const {
            return strTagName;
        }
        typeChar getTagValue(void) const {
            return strTagValue;
        }
        typeChar getTagAttribute(const typeChar &key) const {
            if (mAttributeMap.find(key) == mAttributeMap.end())
                return typeChar("");
            return mAttributeMap[key];
        }
    };

public:
    TXmlConfigureManager(const tstring &strConfDir);

public:
    virtual bool init();
    typedef xmlnode<char> TXmlNode;
    typedef std::map<tstring, TXmlNode> TFilesListMap;

private:
    TFilesListMap m_mKVMap;
private:
    bool initIniFiles();
    void initSingerFile(TXmlNode &vNode, const tstring &confPath);
};

NAMESPACE_END

#endif  // !_X_XMLCONFMGR_H_2020_04_28
