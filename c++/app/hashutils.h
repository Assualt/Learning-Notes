#include <iostream>
#include <string.h>
#include "base64.hpp"
#include <zlib.h>
#define MAX_BUF_SIZE 8192
class HashUtils
{
public:
    static int EncodeBase64(const std::string &src, std::string &dest);
    static int DecodeBase64(const std::string &src, std::string &dest);
    static int GZipDecompress(std::stringstream &in, std::stringstream &out);
    static int GZipDEcompress(const char *buffer, size_t nBytes, stringstream &bufStream);
};

int HashUtils::EncodeBase64(const std::string &src, std::string &dest)
{
    base64::encoder b64;
    std::stringstream in;
    in << src;
    std::ostringstream out;
    b64.encode(in, out);
    dest = out.str();
    dest = dest.substr(0, dest.size() - 1);
    return dest.size();
}

int HashUtils::DecodeBase64(const std::string &src, std::string &dest)
{
    base64::decoder b64;
    std::stringstream in;
    in << src;
    std::ostringstream out;
    b64.decode(in, out);
    dest = out.str();
    return dest.size();
}
int HashUtils::GZipDEcompress(const char *buffer, size_t nBytes, stringstream &bufStream)
{
    if(buffer == nullptr || nBytes == 0)
        return 0;
    z_stream strm;
    char outBuf[MAX_BUF_SIZE];
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int left;
    int ret;
    int size = 0;
    if ((ret = inflateInit2(&strm, 47)) != Z_OK)
    {
        inflateEnd(&strm);
        return -1;
    }
    do
    {
        strm.avail_in = nBytes;
        strm.next_in = (Bytef *)buffer;
        do
        {
            strm.avail_out = MAX_BUF_SIZE;
            strm.next_out = (Bytef *)outBuf;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR)
            {
                inflateEnd(&strm);
                return -2;
            }
            switch (ret)
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR; /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return -1;
            }
            left = MAX_BUF_SIZE - strm.avail_out;
            if (outBuf != nullptr)
            {
                bufStream.write(outBuf, left);
                size += left;
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);
    return size;
}

int HashUtils::GZipDecompress(std::stringstream &in, std::stringstream &out)
{
    int size = 0;
    z_stream strm;
    char inBuf[MAX_BUF_SIZE], outBuf[MAX_BUF_SIZE];
    int ret;
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int left;
    if ((ret = inflateInit2(&strm, 47)) != Z_OK)
    {
        inflateEnd(&strm);
        return -1;
    }
    do
    {
        memset(inBuf, 0, MAX_BUF_SIZE);
        off_t nPos = in.tellg();
        std::cout << "npos:" << nPos << std::endl;
        strm.avail_in = in.readsome(inBuf, sizeof(inBuf));
        std::cout << "npos:" << strm.avail_in << std::endl;
        if (inBuf == nullptr)
            break;
        strm.next_in = (Bytef *)inBuf;
        do
        {
            strm.avail_out = MAX_BUF_SIZE;
            strm.next_out = (Bytef *)outBuf;
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR)
            {
                inflateEnd(&strm);
                return -2;
            }
            switch (ret)
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR; /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return -1;
            }
            left = MAX_BUF_SIZE - strm.avail_out;
            if (outBuf != nullptr)
            {
                out.write(outBuf, left);
                size += left;
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    return size;
}