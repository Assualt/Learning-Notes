//
// Created by 侯鑫 on 2024/1/1.
//

#include "file.h"
#include <unistd.h>
#include <utility>
#include "system.h"

using namespace ssp::base;

namespace {
constexpr size_t maxLintBufferSize = 1024; // buffer缓冲区大小
}

File::File(int32_t fd, std::string errMsg)
    : fd_(fd), errMsg_(std::move(errMsg))
{
}

File::~File() noexcept
{
    Close();
}

std::string File::GetErr() const
{
    return errMsg_;
}


void File::Close() const noexcept
{
    if (fd_ == -1) {
        return;
    }

    close(fd_);
}

File File::New(const std::string &file, int32_t flag, mode_t mode)
{
    int32_t fd = open(file.c_str(), flag, mode);
    if (fd == -1) {
        return File{-1, System::GetErrMsg(errno) };
    }

    return File{fd, ""};
}

[[maybe_unused]] File::File(std::string_view path, int32_t flag, mode_t mode)
{
    fd_ = open(path.data(), flag, mode);
    errMsg_ = System::GetErrMsg(errno);
}

int32_t File::Write(void *buffer, size_t size) const
{
    if (buffer == nullptr || size == 0) {
        return 0;
    }

    if (fd_ == -1) {
        return 0;
    }

    return static_cast<int32_t>(write(fd_, buffer, size));
}

int32_t File::ReadLine(std::string &line)
{
    if (fd_ == -1) {
        return -1;
    }

    auto tempBuffer = std::make_unique<uint8_t[]>(maxLintBufferSize);
    if (tempBuffer == nullptr) {
        errMsg_ = "can't malloc temp buffer.";
        return -1;
    }

    int32_t readNum = 0;

    while (true) {
        do {
            memset(tempBuffer.get(), 0, maxLintBufferSize);
            ssize_t readBytes = read(fd_, (void *) tempBuffer.get(), maxLintBufferSize);
            if (readBytes == 0) {
                goto end;
            }

            if (readBytes == -1) {
                errMsg_ = System::GetErrMsg(errno);
                return static_cast<int32_t>(readBytes);
            }

            for (uint32_t idx = 0; idx < readBytes; ++idx) {
                if (tempBuffer[idx] == '\n') {
                    goto end;
                }

                line.push_back((char) tempBuffer[idx]);
                readNum++;
            }
        } while (errno == EINTR);
    }

end:
    return readNum;
}

void File::Flush() const
{
    if (fd_ == -1) {
        return;
    }

    fsync(fd_);
}

File File::Temporary()
{
    FILE *tmpFile = tmpfile();
    if (tmpFile == nullptr) {
        return File{-1, System::GetErrMsg(errno)};
    }
    int fd = ::dup(fileno(tmpFile));
    fclose(tmpFile);
    return File(fd, "");
}

FileAttr File::GetFileAttr()
{
    if (fd_ == -1) {
        return {};
    }

    struct stat st{};
    if(fstat(fd_, &st) == -1) {
        errMsg_ = System::GetErrMsg(errno);
        return {};
    }
    
    FileAttr attr;
    if (S_ISBLK(st.st_mode)) {
        attr.fileType = FileType::BLOCK;
    } else if (S_ISREG(st.st_mode)) {
        attr.fileType = FileType::REG_FILE;
    } else if (S_ISCHR(st.st_mode)) {
        attr.fileType = FileType::CHAR;
    } else if (S_ISDIR(st.st_mode)) {
        attr.fileType = FileType::DIR;
    } else if (S_ISLNK(st.st_mode)) {
        attr.fileType = FileType::LINK;
    } else if (S_ISSOCK(st.st_mode)) {
        attr.fileType = FileType::SOCKET;
    } else if (S_ISFIFO(st.st_mode)) {
        attr.fileType = FileType::PIPE;
    } else {
        attr.fileType = FileType::UNKNOWN;
    }

    attr.modifyTime = TimeStamp::FromUnixTime(st.st_mtimespec.tv_sec);
    attr.accessTime = TimeStamp::FromUnixTime(st.st_atimespec.tv_sec);
    attr.createTime = TimeStamp::FromUnixTime(st.st_birthtimespec.tv_sec);
    attr.uid = st.st_uid;
    attr.gid = st.st_gid;
    attr.size = st.st_size;
    return attr;
}