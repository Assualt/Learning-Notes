#include "File.h"
#include <sys/file.h>
#include <unistd.h>
using namespace muduo;
using namespace muduo::base;

File::File() noexcept
    : m_nFd(-1)
    , m_nOwnFd(false) {
}

File::File(int fd, bool ownsFd) noexcept
    : m_nFd(fd)
    , m_nOwnFd(ownsFd) {
}

File::File(const char *name, int flags, mode_t mode) {
    m_nFd = ::open(name, flags, mode);
    if (m_nFd == -1) {
        // throw exception
        throw FileException("open file error");
    }
    m_nOwnFd = true;
}

File::File(const std::string &name, int flags, mode_t mode)
    : File(name.c_str(), flags, mode) {
}

File::File(StringPiece name, int flags, mode_t mode)
    : File(name.data(), flags, mode) {
}

File::File(File &&rhs) noexcept
    : m_nFd(rhs.m_nFd)
    , m_nOwnFd(rhs.m_nOwnFd) {
    rhs.Release();
}

File &File::operator=(File &&rhs) {
    CloseNoThrow();
    Swap(rhs);
    return *this;
}

File::~File() {
    auto fd = m_nFd;
    CloseNoThrow();
}

File File::Temporary() {
    FILE *tmpFile = tmpfile();
    if (tmpFile == nullptr) {
        // throw open file error
        throw FileException("open file with nullptr pointer");
    }
    fclose(tmpFile);
    int fd = ::dup(fileno(tmpFile));
    return File(fd, true);
}

int File::Release() noexcept {
    int released = m_nFd;
    m_nFd        = -1;
    m_nOwnFd     = false;
    return released;
}

void File::Swap(File &other) noexcept {
    using std::swap;
    swap(m_nFd, other.m_nFd);
    swap(m_nOwnFd, other.m_nOwnFd);
}

void swap(File &a, File &b) noexcept {
    a.Swap(b);
}

File File::Dup() const {
    if (m_nFd != -1) {
        int fd = ::dup(m_nFd);
        if (fd == -1) {
            // throw error
            throw FileException("dup file error");
        }
        return File(fd, true);
    }
    return File();
}

void File::Close() {
    if (!CloseNoThrow()) {
        // throwSystemError("close() failed");
        throw std::runtime_error("close() failed");
    }
}

bool File::CloseNoThrow() {
    int r = m_nOwnFd ? ::close(m_nFd) : 0;
    Release();
    return r == 0;
}

void File::Lock() {
    DoLock(LOCK_EX);
}

bool File::TryLock() {
    return DoTryLock(LOCK_EX);
}

void File::LockShared() {
    DoLock(LOCK_SH);
}

bool File::TryLockShared() {
    return DoTryLock(LOCK_SH);
}

void File::DoLock(int op) {
    int ret = flock(m_nFd, op);
    if (ret != 0)
        throw std::runtime_error("flock() failed lock");
}

bool File::DoTryLock(int op) {
    int r = flock(m_nFd, op | LOCK_NB);
    // flock returns EWOULDBLOCK if already locked
    if (r == -1 && errno == EWOULDBLOCK) {
        return false;
    }
    if (r != 0)
        throw std::runtime_error("flock() failed try_lock");
    return true;
}

void File::UnLock() {
    int ret = flock(m_nFd, LOCK_UN);
    if (ret != 0)
        throw std::runtime_error("flock() failed try_lock");
}

void File::UnLockShared() {
    UnLock();
}

// Read
std::string File::ReadLineByChar(char ch) {
    char        temp[ 2 ];
    std::string lineString;
    ssize_t     nRead;
    while (1) {
        nRead = read(m_nFd, temp, 1);
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
    return read(m_nFd, temp, nBytes);
}