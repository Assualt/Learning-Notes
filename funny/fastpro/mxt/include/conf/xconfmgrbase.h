#ifndef _X_CONFMGRBASE_H_2018_12_14_
#define _X_CONFMGRBASE_H_2018_12_14_
#include <base/xstring.h>

#include <algorithm>
#include <map>
#include <list>
#include <vector>
#include <base/xexception.h>

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

NAMESPACE_BEGIN

DECLARE_EXCEPTION(TConfigureException, XException);

class TConfigureManagerBase
{
public:
  enum CONF_TYPE
  {
    TYPE_UNKNOWN,
    TYPE_INI,
    TYPE_XML,
    TYPE_JSON,
    TYPE_YML
  };

public:
  TConfigureManagerBase(const tstring &strConfDir) : m_strConfigureDir(strConfDir),m_nConfType(TConfigureManagerBase::TYPE_INI)
  {
    ScanFilePath(m_strConfigureDir);
  }
  TConfigureManagerBase(const tstring &strConfDir, TConfigureManagerBase::CONF_TYPE strSurffix)
      : m_strConfigureDir(strConfDir), m_nConfType(strSurffix)
  {
    ScanFilePath(m_strConfigureDir);
  }
  virtual bool onInit() = 0;

  bool ChangeAccessPath(const tstring &strConfDir)
  {
    m_strConfigureDir = strConfDir;
    m_strFileNames.clear();
    ScanFilePath(m_strConfigureDir);
    return true;
  }
protected:
  tstring m_strConfigureDir;           //conf的目录
  CONF_TYPE m_nConfType;               //conf的后缀
  std::vector<tstring> m_strFileNames; //文件目录
private:
  tstring getConfType()
  {
    int type = static_cast<int>(m_nConfType);
    tstring strType;
    switch (type)
    {
    case TYPE_UNKNOWN:
      strType = "";
      break;
    case TYPE_INI:
      strType = ".ini";
      break;
    case TYPE_XML:
      strType = ".xml";
      break;
    case TYPE_JSON:
      strType = ".json";
      break;
    case TYPE_YML:
      strType = ".yml";
      break;
    default:
      strType = ".ini";
      break;
    }
    return strType;
  }
  void ScanFilePath(const tstring &strDirName)
  {
    if (strDirName.empty())
      return;
    struct stat st;
    if (stat(strDirName.c_str(), &st) == -1)
      throw new TConfigureException(TFmtstring("config dir is not found. stat error.msg:%").arg(strerror(errno)).c_str(), XException::XEP_ERROR);

    DIR *dir;
    if ((dir = opendir(m_strConfigureDir.c_str())) != NULL)
    {
      struct dirent *cur_dirent;
      while ((cur_dirent = readdir(dir)) != NULL)
      {
        if (strcmp(cur_dirent->d_name,".") == 0 || strcmp(cur_dirent->d_name,"..") == 0)
          continue;
        tstring strTmpFile = TFmtstring("%/%").arg(strDirName).arg(cur_dirent->d_name).c_str();
        struct stat tmp;
        stat(strTmpFile.c_str(), &tmp);
        switch (tmp.st_mode & S_IFMT)
        {
        case S_IFREG:
          if (TStringHelper::endWith(strTmpFile, getConfType().c_str()))
            m_strFileNames.push_back(strTmpFile);
          break;
        case S_IFDIR:
          ScanFilePath(strTmpFile);
          break;
        default:
          break;
        }
      }
    }
  }
};

class TInIConfigureManager : public TConfigureManagerBase
{
public:
  TInIConfigureManager(const tstring &strConfDir) : TConfigureManagerBase(strConfDir, CONF_TYPE::TYPE_INI) {}

public:
  bool onInit();

};

NAMESPACE_END

#endif //!_X_CONFMGRBASE_H_2018_12_14_