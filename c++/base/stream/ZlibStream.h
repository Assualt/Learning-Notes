#pragma once
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <zlib.h>
#define MAX_BUF_SIZE 8192
namespace muduo::base {

class MyStringBuffer : public std::stringbuf {
public:
    void seekReadPos(ssize_t nPos) { std::stringbuf::seekpos(nPos, std::ios_base::in); }

    std::string toString() {
        std::string temp;
        seekReadPos(0);
        do {
            temp.push_back(this->sgetc());
        } while (this->snextc() != EOF);
        seekReadPos(0);
        return temp;
    }

    void seekWritePos(ssize_t nPos) { std::stringbuf::seekpos(nPos, std::ios_base::out); }

    void clear() { std::stringbuf::setbuf((char *)"", std::streamsize(0)); }

    size_t size() {
        seekReadPos(0);
        size_t ret = 0;
        while (this->snextc() != EOF) {
            ret += 1;
        }
        seekReadPos(0);
        return ret;
    }

    void toFile(const std::string &path) {
        std::ofstream fout(path.c_str(), std::ios_base::binary);
        seekReadPos(0);
        int ch;
        while ((ch = this->snextc()) != EOF) {
            fout << ch;
        }
        seekReadPos(0);
        fout.close();
    }
};

class ZlibStream {
    static uLongf DecompressWithZlib(MyStringBuffer &in, MyStringBuffer &out, int nLevel) {
        z_stream strm;
        strm.zalloc   = Z_NULL;
        strm.zfree    = Z_NULL;
        strm.opaque   = Z_NULL;
        strm.avail_in = Z_NULL;
        int ret       = Z_OK;
        // Init with level
        if (nLevel == 1) { // gzip
            ret = inflateInit2(&strm, MAX_WBITS + 16);
        } else if (nLevel == 2) { // deflate
            ret = inflateInit2(&strm, -MAX_WBITS);
        } else if (nLevel == 3) {
            ret = inflateInit(&strm);
        }
        if (ret != Z_OK) {
            inflateEnd(&strm);
            return -1;
        }

        char inBuf[ MAX_BUF_SIZE ]  = {0};
        char outBuf[ MAX_BUF_SIZE ] = {0};
        int  left, size = 0;
        do {
            (void)memset(inBuf, 0, MAX_BUF_SIZE);
            size_t nRead = in.sgetn(inBuf, MAX_BUF_SIZE);
            if (nRead == 0) {
                break;
            }
            strm.avail_in = nRead;
            strm.next_in  = (Bytef *)inBuf;
            do {
                strm.avail_out = MAX_BUF_SIZE;
                strm.next_out  = (Bytef *)outBuf;
                ret            = inflate(&strm, Z_NO_FLUSH);

                if (ret == Z_STREAM_ERROR) {
                    inflateEnd(&strm);
                    return -2;
                }
                switch (ret) {
                    case Z_NEED_DICT:
                        ret = Z_DATA_ERROR;
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        (void)inflateEnd(&strm);
                        return -1;
                    default:
                        break;
                }
                left = MAX_BUF_SIZE - strm.avail_out;
                if (left != 0) {
                    out.sputn(outBuf, left);
                    size += left;
                }
            } while (strm.avail_out == 0);
        } while (ret != Z_STREAM_END);
        return size;
    }

    static uLongf CompressWithZlib(void *buffer, ssize_t size, MyStringBuffer &out, int nLevel) {
        if (buffer == nullptr || size == 0)
            return 0;
        z_stream strm;
        char     outBuf[ MAX_BUF_SIZE ];
        memset(outBuf, 0, MAX_BUF_SIZE);
        strm.zalloc   = Z_NULL;
        strm.zfree    = Z_NULL;
        strm.opaque   = Z_NULL;
        strm.avail_in = size;
        int ret, left, nTotalSize = 0;
        if (nLevel == 3) { // zlib
            ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
        } else if (nLevel == 2) { // raw
            ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
        } else if (nLevel == 1) { // gzip
            ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
        } else {
            return 0;
        }
        if (ret != Z_OK)
            return 0;
        strm.next_in = (Bytef *)buffer;
        do {
            strm.avail_out = MAX_BUF_SIZE;
            strm.next_out  = (Bytef *)outBuf;
            ret            = deflate(&strm, Z_FINISH);
            if (ret == Z_STREAM_ERROR) {
                (void)deflateEnd(&strm);
                return 0;
            }
            left = MAX_BUF_SIZE - strm.avail_out;
            if (left != 0) {
                out.sputn(outBuf, left);
                nTotalSize += left;
            }
        } while (strm.avail_out == 0);
        (void)deflateEnd(&strm);
        return nTotalSize;
    }

public:
    static uLongf GzipCompress(void *buffer, ssize_t size, MyStringBuffer &out) { // gzip
        return CompressWithZlib(buffer, size, out, 1);
    }

    static uLongf DeflateCompress(void *buffer, ssize_t size, MyStringBuffer &out) { // raw -> deflate
        return CompressWithZlib(buffer, size, out, 2);
    }

    static uLongf ZlibCompress(void *buffer, ssize_t size, MyStringBuffer &out) { // zlib
        return CompressWithZlib(buffer, size, out, 3);
    }

    static uLongf GzipDecompress(MyStringBuffer &in, MyStringBuffer &out) { return DecompressWithZlib(in, out, 1); }

    static uLongf DeflateDecompress(MyStringBuffer &in, MyStringBuffer &out) { return DecompressWithZlib(in, out, 2); }

    // zlib
    static uLongf ZlibDeCompress(MyStringBuffer &in, MyStringBuffer &out) { return DecompressWithZlib(in, out, 3); }
};
} // namespace muduo::base