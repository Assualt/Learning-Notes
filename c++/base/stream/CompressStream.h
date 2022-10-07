#pragma once
#include "BinaryStream.h"
#include "base/crypto/base64.h"
#include <memory>

namespace muduo::base {

enum RET : uint32_t {
    COMPRESS_ERR,
    UNCOMPRESS_ERR,
    OUT_OF_RANGE_ERR,
    BUF_INTERNAL_ERR,
};

template <class Byte = char> class EncoderImpl {
public:
    virtual uint32_t encode(const Byte *, size_t, Byte *) = 0;
};

template <class Byte = char> class DecoderImpl {
public:
    virtual uint32_t decode(const Byte *, size_t, Byte *) = 0;
};

struct Header {
    uint32_t compress_len_; // 压缩字节的长度
    uint32_t origin_len_;   // 压缩前字节长度
};

template <class Encoder, class Decoder, class Byte> class CompressStreamImpl : public BinaryStream {
public:
    uint32_t writeBytes(const Byte *ptr, size_t nBytes) {
        if ((ptr == nullptr) || (nBytes == 0)) {
            return RET::OUT_OF_RANGE_ERR;
        }

        auto buffer = std::make_unique<Byte[]>(2 * nBytes);
        auto size   = encoder_.encode(ptr, nBytes, buffer.get());
        if (size == 0) {
            return RET::COMPRESS_ERR;
        }

        Header header = {.compress_len_ = size, .origin_len_ = nBytes};
        write((const void *)&header, sizeof(header));
        return write((const void *)buffer.get(), size);
    }

    uint32_t readBlock(Byte *out, size_t nBlock = 1) {
        size_t cnt    = nBlock;
        size_t curPos = 0;
        while (cnt--) {
            Header header{};
            auto   size = read((void *)&header, sizeof(header));
            if (size != sizeof(header)) {
                return RET::OUT_OF_RANGE_ERR;
            }

            auto readBuf = std::make_unique<Byte[]>(header.compress_len_);
            size         = read((void *)readBuf.get(), header.compress_len_);
            if (size != header.compress_len_) {
                return RET::BUF_INTERNAL_ERR;
            }
            auto outBuf = std::make_unique<Byte[]>(header.origin_len_);
            size        = decoder_.decode(readBuf.get(), header.compress_len_, outBuf.get());
            if (size != header.origin_len_) {
                return RET::UNCOMPRESS_ERR;
            }

            (void)memcpy(out + curPos, outBuf.get(), header.origin_len_);
            curPos += header.origin_len_;
        }
        return curPos;
    }

private:
    Encoder encoder_;
    Decoder decoder_;
};

class Base64Impl : EncoderImpl<char>, DecoderImpl<char> {
public:
    uint32_t encode(const char *ptr, size_t len, char *out) override { return encoder_.encode_str(ptr, len, out); }

    uint32_t decode(const char *ptr, size_t len, char *out) override { return decoder_.decode_str(ptr, len, out); }

private:
    base64::encoder encoder_;
    base64::decoder decoder_;
};

using Base64Stream = CompressStreamImpl<Base64Impl, Base64Impl, char>;
} // namespace muduo::base