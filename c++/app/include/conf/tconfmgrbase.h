#ifndef _X_CONFMGRBASE_H_2018_12_14_
#define _X_CONFMGRBASE_H_2018_12_14_
#include <algorithm>
#include <map>
#include <list>
#include <vector>
#include <fstream>

#include <base/texception.h>
#include <base/tstring.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

NAMESPACE_BEGIN

DECLARE_EXCEPTION(TConfigureException, TException);

class TConfigureManagerBase {
public:
    enum CONF_TYPE { TYPE_UNKNOWN, TYPE_INI, TYPE_XML, TYPE_JSON, TYPE_YML };

public:
    TConfigureManagerBase(const tstring &strConfDir, CONF_TYPE nType);
    virtual bool init() = 0;
    bool ChangeAccessPath(const tstring &strConfDir);

protected:
    tstring m_strConfigureDir;            // conf的目录
    CONF_TYPE m_nConfType;                // conf的后缀
    std::vector<tstring> m_strFileNames;  //文件目录
private:
    void ScanFilePath(const tstring &strDirName);

protected:
    const std::vector<tstring> &getAllConfFiles() const;
    tstring getConfType();
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
    tstring getBaseString(tstring nDefault, const tstring &strPrefix, const tstring &strSuffix, bool &isDefault);

public:
    // getMethod
    /**
     * @param: strPrefix filePrefix
     * @param: strSuffix section/key
     */
    int getInt(int nDefault, const tstring &strPrefix, const tstring &strSuffix);
    bool getBool(bool nDefault, const tstring &strPrefix, const tstring &strSuffix);
    float getFloat(float nDefault, const tstring &strPrefix, const tstring &strSuffix);
    double getDouble(double nDefault, const tstring &strPrefix, const tstring &strSuffix);
    tstring getString(tstring nDefault, const tstring &strPrefix, const tstring &strSuffix);

protected:
    TFilesListMap m_mKVMap;
};

NAMESPACE_END

#endif  //!_X_CONFMGRBASE_H_2018_12_14_