#ifndef _X_CONFMGRBASE_H_2018_12_14_
#define _X_CONFMGRBASE_H_2018_12_14_
#include <base/xstring.h>

#include <algorithm>
#include <map>
#include <list>
#include <vector>
#include <base/xexception.h>
#include <fstream>

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

NAMESPACE_BEGIN

DECLARE_EXCEPTION(TConfigureException, XException);

class TConfigureManagerBase {
public:
    enum CONF_TYPE { TYPE_UNKNOWN, TYPE_INI, TYPE_XML, TYPE_JSON, TYPE_YML };

public:
    TConfigureManagerBase(const tstring &strConfDir);

    TConfigureManagerBase(const tstring &strConfDir, CONF_TYPE nType);

    virtual bool init();

    bool ChangeAccessPath(const tstring &strConfDir);

protected:
    tstring m_strConfigureDir;            // conf的目录
    CONF_TYPE m_nConfType;                // conf的后缀
    std::vector<tstring> m_strFileNames;  //文件目录
private:
    tstring getConfType();

    void ScanFilePath(const tstring &strDirName);

protected:
    const std::vector<tstring> &getAllConfFiles() const;
};

class TInIConfigureManager : public TConfigureManagerBase {
public:
    TInIConfigureManager(const tstring &strConfDir);

public:
    virtual bool init();
    typedef std::map<tstring, std::map<tstring, tstring>> TFileListMap;
    typedef std::map<tstring, TFileListMap> TFilesListMap;

private:
    bool initIniFiles();

    void initSingerFile(TFileListMap &t, const tstring &confPath);

protected:
    TFilesListMap m_mKVMap;
};

NAMESPACE_END

#endif  //!_X_CONFMGRBASE_H_2018_12_14_