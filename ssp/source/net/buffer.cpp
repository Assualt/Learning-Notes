//
// Created by 侯鑫 on 2024/1/10.
//

#include "buffer.h"
#include "endian.h"
#include <cstring>
#include <string>
#include <cassert>
#include <sys/uio.h>
#ifdef USE_SSL
#include <openssl/ssl.h>
#endif

using namespace ssp::net;

constexpr auto g_ctrl = "\r\n";

void Buffer::swap(Buffer &rhs) {
    m_vBuffer.swap(rhs.m_vBuffer);
    std::swap(m_nReadIndex, rhs.m_nReadIndex);
    std::swap(m_nWriteIndex, rhs.m_nWriteIndex);
}

size_t Buffer::readableBytes() const 
{ 
    return m_nWriteIndex - m_nReadIndex; 
}

size_t Buffer::writeableBytes() const 
{
    if (m_vBuffer.size() - m_nWriteIndex < 0) {
        return 0;
    }
    return m_vBuffer.size() - m_nWriteIndex;
}

size_t Buffer::prependableBytes() const 
{ 
    return m_nReadIndex; 
}

const char *Buffer::begin() const 
{
    return &*m_vBuffer.begin(); 
}

char *Buffer::begin() 
{
    return &*m_vBuffer.begin(); 
}

const char *Buffer::peek() const 
{
    return begin() + m_nReadIndex; 
}

const char *Buffer::beginWrite() const 
{
    return begin() + m_nWriteIndex; 
}

char *Buffer::beginWrite() 
{
    return begin() + m_nWriteIndex; 
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstring-plus-int"

const char *Buffer::findCRLF() const 
{
    const char *crlf = std::search(peek(), beginWrite(), g_ctrl, g_ctrl + 2);
    return crlf == beginWrite() ? nullptr : crlf;
}

const char *Buffer::findCRLF(const char *start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    // FIXME: replace with memmem()?
    const char *crlf = std::search(start, beginWrite(), g_ctrl, g_ctrl + 2);
    return crlf == beginWrite() ? NULL : crlf;
}

#pragma clang diagnostic pop

const char *Buffer::findEOL() const {
    const void *eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char *>(eol);
}

const char *Buffer::findEOL(const char *start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void *eol = memchr(start, '\n', beginWrite() - start);
    return static_cast<const char *>(eol);
}

void Buffer::retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len <= readableBytes())
        m_nReadIndex += len;
    else
        retrieveAll();
}

void Buffer::retrieveAll() {
    m_nReadIndex  = m_nCheapPrepend;
    m_nWriteIndex = m_nCheapPrepend;
}

void Buffer::retrieveInt64() { retrieve(sizeof(int64_t)); }

void Buffer::retrieveInt32() { retrieve(sizeof(int32_t)); }

void Buffer::retrieveInt16() { retrieve(sizeof(int16_t)); }

void Buffer::retrieveInt8() { retrieve(sizeof(int8_t)); }

std::string Buffer::retrieveAllAsString() 
{ 
    return retrieveAsString(readableBytes()); 
}

void Buffer::hasWritten(size_t len) 
{
    assert(len <= writeableBytes());
    m_nWriteIndex += len;
}

void Buffer::unwrite(size_t len) 
{
    assert(len <= readableBytes());
    m_nWriteIndex -= len;
}

std::string Buffer::retrieveAsString(size_t len) 
{
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

Buffer &Buffer::append(const char *data) 
{
    append(data, strlen(data));
    return *this;
}

Buffer &Buffer::append(const std::string &buffer) 
{
    append(buffer.data(), buffer.size());
    return *this;
}

Buffer &Buffer::append(const char *data, size_t len) 
{
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
    return *this;
}

Buffer &Buffer::append(const void * /*restrict*/ data, size_t len)
{
    append(static_cast<const char *>(data), len);
    return *this;
}

Buffer &Buffer::appendInt64(int64_t x) 
{
    auto be = sockets::HostToNetwork64(x);
    return append(&be, sizeof(be));
}

Buffer &Buffer::appendInt32(int32_t x) 
{
    auto be = sockets::HostToNetwork32(x);
    return append(&be, sizeof(be));
}

Buffer &Buffer::appendInt16(int16_t x) 
{
    auto be = sockets::HostToNetwork16(x);
    return append(&be, sizeof(be));
}

Buffer &Buffer::appendInt8(int8_t x) 
{ 
    return append(&x, sizeof(x)); 
}

int64_t Buffer::peekInt64() const 
{
    assert(readableBytes() >= sizeof(int64_t));
    int64_t result = 0;
    ::memcpy(&result, peek(), sizeof(result));
    return sockets::NetworkToHost64(result);
}

int32_t Buffer::peekInt32() const 
{
    assert(readableBytes() >= sizeof(int32_t));
    int32_t result = 0;
    ::memcpy(&result, peek(), sizeof(result));
    return sockets::NetworkToHost32(result);
}

int16_t Buffer::peekInt16() const
{
    assert(readableBytes() >= sizeof(int16_t));
    int16_t result = 0;
    ::memcpy(&result, peek(), sizeof(result));
    return sockets::NetworkToHost16(result);
}

int8_t Buffer::peekInt8() const 
{
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
}

int64_t Buffer::readInt64() 
{
    int64_t result = peekInt64();
    retrieveInt64();
    return result;
}

int32_t Buffer::readInt32() 
{
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
}

int16_t Buffer::readInt16() 
{
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
}

int8_t Buffer::readInt8()
{
    int8_t result = (peekInt8() & UINT8_MAX);
    retrieveInt8();
    return result;
}

void Buffer::prependInt64(int64_t x) 
{
    int64_t be64 = sockets::HostToNetwork64(x);
    prepend(&be64, sizeof(be64));
}

///
/// Prepend int32_t using network endian
///
void Buffer::prependInt32(int32_t x) 
{
    int32_t be32 = sockets::HostToNetwork32(x);
    prepend(&be32, sizeof(be32));
}

void Buffer::prependInt16(int16_t x) 
{
    int16_t be16 = sockets::HostToNetwork16(x);
    prepend(&be16, sizeof(be16));
}

void Buffer::prependInt8(int8_t x) 
{
    prepend(&x, sizeof(x)); 
}

void Buffer::prepend(const void * /*restrict*/ data, size_t len) 
{
    assert(len <= prependableBytes());
    m_nReadIndex -= len;
    const char *d = static_cast<const char *>(data);
    std::copy(d, d + len, begin() + m_nReadIndex);
}

void Buffer::ensureWritableBytes(size_t len) 
{
    if (writeableBytes() < len)
        makeSpace(len);
    // assert(readableBytes() >= len);
}

void Buffer::makeSpace(size_t len) 
{
    if (writeableBytes() + prependableBytes() < len + m_nCheapPrepend) 
    {
        m_vBuffer.resize(m_nWriteIndex + len);
        return;
    }
    assert(m_nCheapPrepend < m_nReadIndex);
    size_t readable = readableBytes();
    std::copy(begin() + m_nReadIndex, begin() + m_nWriteIndex, begin() + prependableBytes());
    m_nReadIndex  = m_nCheapPrepend;
    m_nWriteIndex = m_nReadIndex + readable;
    assert(readable == readableBytes());
}

void Buffer::shrink(size_t reserve) 
{
    m_vBuffer.shrink_to_fit();
}

size_t Buffer::internalCapacity() const 
{
    return m_vBuffer.capacity(); 
}

/// Read data directly into buffer.
///
/// It may implement with readv(2)
/// @return result of read(2), @c errno is saved
ssize_t Buffer::readFd(int fd, int *savedErrno) 
{
    // saved an ioctl()/FIONREAD call to tell how much to read
    char         exBuf[ 65536 ] = {0};
    struct iovec vec[ 2 ];
    const size_t writeable = writeableBytes();
    vec[ 0 ].iov_base      = begin() + m_nWriteIndex;
    vec[ 0 ].iov_len       = writeable;
    vec[ 1 ].iov_base      = exBuf;
    vec[ 1 ].iov_len       = sizeof(exBuf);
    // when there is enough space in this buffer, don't read into exBuf.
    // when exBuf is used, we read 128k-1 bytes at most.
    const int iovCnt = (writeable < sizeof(exBuf)) ? 2 : 1;

    const ssize_t n = ::readv(fd, vec, iovCnt);
    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writeable) {
        m_nWriteIndex += n;
    } else {
        m_nWriteIndex = m_vBuffer.size();
        append(exBuf, n - writeable);
    }
    return n;
}

void Buffer::retrieveUntil(const char *end) 
{
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}
