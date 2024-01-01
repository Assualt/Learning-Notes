//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_FILE_H
#define SSP_TOOLS_FILE_H

#include <cstdint>
#include <sys/stat.h>
#include <string>
#include <fcntl.h>
#include <iostream>
#include <map>
#include "timestamp.h"

namespace ssp::base {

enum FileOpType : int32_t {
    READ    =       O_RDONLY,
    WRITE   =       O_WRONLY,
    RDWR    =       O_RDWR,
    CREATE  =       O_CREAT,
    TRUNC   =       O_TRUNC,
    EXEC [[maybe_unused]] =       O_EXEC,
    APPEND [[maybe_unused]] =       O_APPEND,
    SEARCH [[maybe_unused]] =       O_SEARCH,
};

enum FileType {
    LINK,
    REG_FILE,
    DIR,
    SOCKET,
    BLOCK,
    PIPE,
    CHAR,
    UNKNOWN
};

inline std::string GetFileType(FileType type)
{
    static std::map<FileType, std::string> file2Map = {
        {LINK, "link"}, {REG_FILE, "file"}, {DIR, "dir"}, {SOCKET, "socket"},
        {BLOCK, "block"}, {PIPE, "pipe"}, {CHAR, "char"}, {UNKNOWN, "unknown"}
    };

    auto iter = file2Map.find(type);
    return iter == file2Map.end() ? file2Map[UNKNOWN] : iter->second;
}

struct FileAttr {
    TimeStamp modifyTime;
    TimeStamp accessTime;
    TimeStamp createTime;
    std::string name;
    FileType fileType;
    std::string absPath;
    uid_t uid;
    gid_t gid;
    off_t size;

public:
    friend std::ostream& operator<<(std::ostream &os, const FileAttr& attr)
    {
        os << "name:" << attr.name << std::endl
           << "name(abs):" << attr.absPath << std::endl
           << "time(Access):" << attr.accessTime.ToFmtString() << std::endl
           << "    (Modify):" << attr.modifyTime.ToFmtString() << std::endl
           << "    (Create):" << attr.accessTime.ToFmtString() << std::endl
           << "uid:" << std::dec << attr.uid << " gid:" << attr.gid << std::endl
           << "type:" << GetFileType(attr.fileType) << std::endl
           << "size(Bytes):" << attr.size << std::endl;
        return os;
    }
};

class File {
public:
    explicit File(int32_t fd, std::string errMsg = "");

    [[maybe_unused]] explicit File(std::string_view path, int32_t flag, mode_t mode = 0666);

    ~File() noexcept;

public:
    void Close() const noexcept;

    static File New(const std::string &file, int32_t flag, mode_t mode);

    static File Temporary();

    int32_t Write(void *buffer, size_t size) const;

    int32_t ReadLine(std::string &line);

    [[nodiscard]] std::string GetErr() const;

    void Flush() const;

    FileAttr GetFileAttr();

private:
    int32_t fd_{-1};
    std::string errMsg_;
    std::string name_;
    std::string absPath_;
};

}

#endif //SSP_TOOLS_FILE_H
