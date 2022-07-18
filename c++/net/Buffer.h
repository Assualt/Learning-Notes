#pragma once

// A simple Fork by muduo's Buffer
#include <algorithm>
#include <assert.h>
#include <base/Range.h>
#include <iostream>
#include <vector>
#define KCRTL "\r\n"

namespace muduo {
namespace net {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
public:
    static const size_t m_nCheapPrepend = 8;
    static const size_t m_nInitialSize  = 1024;

private:
    std::vector<char> m_vBuffer;
    size_t            m_nReadIndex;
    size_t            m_nWriteIndex;

public:
    explicit Buffer(size_t nInitialSize = m_nInitialSize)
        : m_vBuffer(m_nCheapPrepend + nInitialSize)
        , m_nReadIndex(m_nCheapPrepend)
        , m_nWriteIndex(m_nCheapPrepend) {
    }

    void   swap(Buffer &rhs);
    size_t readableBytes() const;
    size_t writeableBytes() const;
    size_t prependableBytes() const;

    const char *peek() const;
    const char *findCRLF() const;
    const char *findCRLF(const char *start) const;
    const char *beginWrite() const;
    char       *beginWrite();

    const char *findEOL() const;
    const char *findEOL(const char *start) const;

    // retrieve returns void, to prevent
    // string str(retrieve(readableBytes()), readableBytes());
    // the evaluation of two functions are unspecified
    void retrieve(size_t len);
    void retrieveAll();

    void retrieveInt64();
    void retrieveInt32();
    void retrieveInt16();
    void retrieveInt8();

    void hasWritten(size_t len);
    void unwrite(size_t len);

    std::string       retrieveAllAsString();
    std::string       retrieveAsString(size_t len);
    base::StringPiece toStringPiece() const;

    Buffer &append(const base::StringPiece &piece);
    Buffer &append(const char *data, size_t len);
    Buffer &append(const void * /*restrict*/ data, size_t len);
    Buffer &append(const std::string &buffer);
    Buffer &append(const char *data);
    Buffer &appendInt64(int64_t x);
    Buffer &appendInt32(int32_t x);
    Buffer &appendInt16(int16_t x);
    Buffer &appendInt8(int8_t x);

    void ensureWritableBytes(size_t len);

    int64_t peekInt64() const;
    int32_t peekInt32() const;
    int16_t peekInt16() const;
    int8_t  peekInt8() const;

    int64_t readInt64();
    int32_t readInt32();
    int16_t readInt16();
    int8_t  readInt8();

    void prepend(const void * /*restrict*/ data, size_t len);
    void prependInt8(int8_t x);
    void prependInt16(int16_t x);
    void prependInt32(int32_t x);
    void prependInt64(int64_t x);

    void    shrink(size_t reserve);
    size_t  internalCapacity() const;
    ssize_t readFd(int fd, int *savedErrno);
    void    retrieveUntil(const char *end);

private:
    const char *begin() const;
    char       *begin();
    void        makeSpace(size_t len);
};

} // namespace net
} // namespace muduo
