#pragma once
#include "base64.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <zlib.h>
#define MAX_BUF_SIZE 8192

class MyStringBuffer : public std::stringbuf {
public:
    void seekReadPos(ssize_t nPos) {
        std::stringbuf::seekpos(nPos, std::ios_base::in);
    }
    std::string toString() {
        std::string temp;
        seekReadPos(0);
        do {
            temp.push_back(this->sgetc());
        } while (this->snextc() != EOF);
        seekReadPos(0);
        return temp;
    }
    void seekWritePos(ssize_t nPos) {
        std::stringbuf::seekpos(nPos, std::ios_base::out);
    }
    void clear() {
        std::stringbuf::setbuf((char *)"", std::streamsize(0));
    }

    size_t size() {
        seekReadPos(0);
        size_t ret = 0;
        while (this->snextc() != EOF) {
            ret += 1;
        }
        return ret;
    }
};

class HashUtils {
public:
    static int EncodeBase64(const std::string &src, std::string &dest) {
        base64::encoder   b64;
        std::stringstream in;
        in << src;
        std::ostringstream out;
        b64.encode(in, out);
        dest = out.str();
        dest = dest.substr(0, dest.size() - 1);
        return dest.size();
    }
    static int DecodeBase64(const std::string &src, std::string &dest) {
        base64::decoder   b64;
        std::stringstream in;
        in << src;
        std::ostringstream out;
        b64.decode(in, out);
        dest = out.str();
        return dest.size();
    }
    static uLongf DecompressWithZlib(MyStringBuffer &in, std::stringstream &out, int nLevel) {
        z_stream strm;
        strm.zalloc   = Z_NULL;
        strm.zfree    = Z_NULL;
        strm.opaque   = Z_NULL;
        strm.avail_in = 0;
        strm.next_in  = Z_NULL;
        // level 
        int ret;
        if(nLevel == 1){// gzip
            ret = inflateInit2(&strm, MAX_WBITS + 16);
        }else if(nLevel == 2){ // deflate
            ret = inflateInit2(&strm, -MAX_WBITS);
        }else if(nLevel == 3){ // raw
            ret = inflateInit(&strm);
        }
        if (ret != Z_OK) {
            inflateEnd(&strm);
            return -1;
        }
        char inBuf[ MAX_BUF_SIZE ], outBuf[ MAX_BUF_SIZE ];
        int  left, size = 0;
        do {
            memset(inBuf, 0, MAX_BUF_SIZE);
            size_t nRead = in.sgetn(inBuf, MAX_BUF_SIZE);
            if (nRead < 0)
                break;
            strm.avail_in = nRead;

            if (inBuf == nullptr)
                break;
            strm.next_in = (Bytef *)inBuf;
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
                        ret = Z_DATA_ERROR; /* and fall through */
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                        (void)inflateEnd(&strm);
                        return -1;
                }
                left = MAX_BUF_SIZE - strm.avail_out;
                if (outBuf != nullptr) {
                    out.write(outBuf, left);
                    size += left;
                }
            } while (strm.avail_out == 0);

        } while (ret != Z_STREAM_END);

        inflateEnd(&strm);
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
            // ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_COMPRESSION);
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
            out.sputn(outBuf, left);
            nTotalSize += left;
        } while (strm.avail_out == 0);
        (void)deflateEnd(&strm);
        return nTotalSize;
    }

    static uLongf GzipCompress(void *buffer, ssize_t size, MyStringBuffer &out) { // gzip
        return CompressWithZlib(buffer, size, out, 1);
    }

    static uLongf DeflateCompress(void *buffer, ssize_t size, MyStringBuffer &out) { // raw -> deflate
        return CompressWithZlib(buffer, size, out, 2);
    }

    static uLongf ZlibCompress(void *buffer, ssize_t size, MyStringBuffer &out) { // zlib
        return CompressWithZlib(buffer, size, out, 3);
    }

    static uLongf GzipDecompress(MyStringBuffer &in, std::stringstream &out) {
        return DecompressWithZlib(in, out,1);
    }

    static uLongf DeflateDecompress(MyStringBuffer &in, std::stringstream &out) {
        return DecompressWithZlib(in, out, 2);
    }

    static uLongf ZlibDeCompress(MyStringBuffer &in, std::stringstream &out) { // zlib
        return DecompressWithZlib(in, out, 3);
    }
};
