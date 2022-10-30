#pragma once
#include "Timestamp.h"
#include "nonecopyable.h"
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
namespace muduo::base {

class FileAttr {
    //!	The attribute flags
    enum TFileFlags {
        type_None = 0x00,
        type_FILE = 0x01,
        type_DIR  = 0x02,
        type_Link = 0x03,
    };

public:
    explicit FileAttr(const std::string &strName = "", const std::string &strParent = "", TFileFlags attr = type_None)
        : m_strParent(strParent)
        , m_strName(strName)
        , m_nFlags(attr)
        , m_nSize(0)
        , m_tCreate(0)
        , m_tModify(0)
        , m_tRead(0) {}

    //! copy constructor
    FileAttr(const FileAttr &rt) = default;

    //! destrcutor
    ~FileAttr() = default;

    //! copy assert operator
    FileAttr &operator=(const FileAttr &rt) {
        if (&rt != this) {
            FileAttr tempVal(rt);
            Swap(tempVal);
        }
        return *this;
    }

    void clear() {
        FileAttr attr;
        Swap(attr);
    }

    void setName(const std::string &strName) {
        clear();
        m_strName = strName;
    }

    //! compare tow file name
    bool operator==(const FileAttr &rt) const { return m_strParent == rt.m_strParent && m_strName == rt.m_strName; }

    //! compare tow file name
    template <class typeFileAttr> bool operator==(const typeFileAttr &rt) const {
        return GetFullName() == rt.GetFullName();
    }

    //! set the parent directory path name
    //! \param boExpandAllInfo	if boExpandAllInfo is true, expand all attribute information, such as create time, last
    //! access time, etc.
    void SetParentPath(const std::string &strParent, bool boExpandAllInfo = true);

public:
    //! return the file/directory name
    const std::string &GetName() const { return m_strName; }

    //! return the parent directory name
    const std::string &SetPath() const { return m_strParent; }

    //! return the file/directory full name
    std::string GetFullName() const { return std::string(m_strParent) += m_strName; }

    std::string BriefName() const {
        std::string name = m_strName;
        return (name.find(".") != std::string::npos ? name.substr(0, name.find(".")) : name);
    }

    //! return the file/directory type.
    TFileFlags GetType() const { return m_nFlags; }

    //! return true when this is a file.
    bool IsFile() const { return (m_nFlags & type_FILE) != 0; }

    //! return true when this is a link.
    bool IsLink() const { return (m_nFlags & type_Link) != 0; }

    //! return true when this is a directory.
    bool IsDir() const { return (m_nFlags & type_DIR) != 0; }

    //! return the create time.
    Timestamp GetCreateTime() const { return m_tCreate; }

    //! return the last modify time.
    Timestamp GetModifyTime() const { return m_tModify; }

    //! return the last access time.
    Timestamp GetReadTime() const { return m_tRead; }

    //! return the file size.
    size_t GetSize() const { return m_nSize; }

    //! swap tow TFileAttr
    void Swap(FileAttr &attr) {
        std::swap(m_strParent, attr.m_strParent);
        std::swap(m_strName, attr.m_strName);
        std::swap(m_nFlags, attr.m_nFlags);
        std::swap(m_nSize, attr.m_nSize);
        std::swap(m_tCreate, attr.m_tCreate);
        std::swap(m_tModify, attr.m_tModify);
        std::swap(m_tRead, attr.m_tRead);
    }

protected:
    std::string m_strParent;
    std::string m_strName;
    TFileFlags  m_nFlags;
    size_t      m_nSize;
    Timestamp   m_tCreate;
    Timestamp   m_tModify;
    Timestamp   m_tRead;
};

class DirScanner : nonecopyable {
public:
    DirScanner(const char *pathName = nullptr);
    ~DirScanner();

public:
    void CloseHandle();
    bool Fetch(FileAttr &attr);
    void StartSearch(const std::string &strPath);

protected:
    DIR *m_nHandle;
};

} // namespace muduo::base