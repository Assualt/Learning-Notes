#pragma once
#include "base/stream/StreamBase.h"

namespace muduo::base {
class CheckSumBase : public WriteStream {
public:
    CheckSumBase()
        : WriteStream(StreamBase::STATE_OPEN) {
    }

    virtual size_t getResultSize() const = 0;
};

class SimpleCheckSum : public CheckSumBase {
public:
    SimpleCheckSum()
        : CheckSumBase()
        , r_(55665)
        , c1_(52845)
        , sum_(0) {
    }

    virtual size_t write(const char *lpBuf, size_t nSize);
    virtual size_t flush();
    size_t         getCheckSum() const;

    void           reset();
    virtual size_t getResultSize() const;

private:
    unsigned short r_;
    unsigned short c1_;
    unsigned short c2_;
    uint32_t       sum_;
};

class CRC32 : public CheckSumBase {
public:
    CRC32()
        : CheckSumBase()
        , sum_(UINT32_MAX) {
    }

    virtual size_t write(const void *lpBuf, size_t nSize);
    virtual size_t flush();
    uint32_t       getChecksum() const;
    void           reset();
    virtual size_t getResultSize() const;

private:
    uint32_t sum_;
};

class MD5 : public CheckSumBase {
public:
    MD5();
    ~MD5();

    virtual size_t write(const void *lpBuf, size_t nSize);
    virtual size_t flush();
    void           reset();

    virtual size_t getResultSize() const;

    const char  *getMD5String() const;
    const tbyte *getMD5Digest() const;

protected:
    void update(const uint8_t *src, uint32_t len);
    void final();

    static void transform(uint32_t state[ 4 ], const uint8_t block[ 64 ]);
    static void encode(uint8_t *output, uint32_t *input, uint32_t len);
    static void decode(uint32_t *output, const uint8_t *input, uint32_t len);

    std::string md5str_;
    tbyte       md5digest_[ 16 ];
    bool        hasFlush_;
    struct MD5_CTX {
        uint32_t state[ 4 ];
        uint32_t count[ 2 ];
        uint8_t  buffer[ 64 ];
    } context_;
};
} // namespace muduo::base