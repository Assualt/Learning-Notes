#include "File.h"
#include <sys/file.h>
#include <unistd.h>
namespace muduo {
namespace base {

File::File() noexcept
    : fd_(-1)
    , ownsfd_(false) {
}

File::File(int fd, bool ownsFd) noexcept
    : fd_(fd)
    , ownsfd_(ownsFd) {
}

File::File(const char *name, int flags, mode_t mode) {
    fd_ = ::open(name, flags, mode);
    if (fd_ == -1) {
        // throw exception
        throw FileException("open file error");
    }
    ownsfd_ = true;
}
File::File(const std::string &name, int flags, mode_t mode)
    : File(name.c_str(), flags, mode) {
}

File::File(StringPiece name, int flags, mode_t mode)
    : File(name.data(), flags, mode) {
}

File::File(File &&rhs) noexcept
    : fd_(rhs.fd_)
    , ownsfd_(rhs.ownsfd_) {
    rhs.release();
}
File &File::operator=(File &&rhs) {
    closeNoThrow();
    swap(rhs);
    return *this;
}

File::~File() {
    auto fd = fd_;
    closeNoThrow();
}

File File::temporary() {
    FILE *tmpFile = tmpfile();
    if (tmpFile == nullptr) {
        // throw open file error
        throw FileException("open file with nullptr pointer");
    }
    fclose(tmpFile);
    int fd = ::dup(fileno(tmpFile));
    return File(fd, true);
}
int File::release() noexcept{
    int released = fd_;
    fd_          = -1;
    ownsfd_      = false;
    return released;
}
void File::swap(File &other) noexcept {
    using std::swap;
    swap(fd_, other.fd_);
    swap(ownsfd_, other.ownsfd_);
}
void swap(File &a, File &b) noexcept {
    a.swap(b);
}

File File::dup() const {
    if (fd_ != -1) {
        int fd = ::dup(fd_);
        if (fd == -1) {
            // throw error
            throw FileException("dup file error");
        }
        return File(fd, true);
    }
    return File();
}
void File::close() {
    if (!closeNoThrow()) {
        // throwSystemError("close() failed");
        throw std::runtime_error("close() failed");
    }
}

bool File::closeNoThrow() {
    int r = ownsfd_ ? ::close(fd_) : 0;
    release();
    return r == 0;
}

void File::lock() {
    doLock(LOCK_EX);
}
bool File::try_lock() {
    return doTryLock(LOCK_EX);
}
void File::lock_shared() {
    doLock(LOCK_SH);
}
bool File::try_lock_shared() {
    return doTryLock(LOCK_SH);
}

void File::doLock(int op) {
    int ret = flock(fd_, op);
    if (ret != 0)
        throw std::runtime_error("flock() failed lock");
}

bool File::doTryLock(int op) {
    int r = flock(fd_, op | LOCK_NB);
    // flock returns EWOULDBLOCK if already locked
    if (r == -1 && errno == EWOULDBLOCK) {
        return false;
    }
    if (r != 0)
        throw std::runtime_error("flock() failed try_lock");
    return true;
}

void File::unlock() {
    int ret = flock(fd_, LOCK_UN);
    if (ret != 0)
        throw std::runtime_error("flock() failed try_lock");
}
void File::unlock_shared() {
    unlock();
}

// Read
std::string File::ReadLineByChar(char ch) {
    char        temp[ 2 ];
    std::string lineString;
    ssize_t     nRead;
    while (1) {
        nRead = read(fd_, temp, 1);
        if (nRead == -1)
            break;
        if (ch == temp[ 0 ]) {
            lineString.push_back(ch);
            break;
        } else {
            lineString.push_back(temp[ 0 ]);
        }
    }
    return lineString;
}

size_t File::ReadBytes(void *temp, size_t nBytes) {
    return read(fd_, temp, nBytes);
}
} // namespace base
} // namespace muduo