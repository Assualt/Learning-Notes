#include "base64.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <zlib.h>
#include "logging.h"
#define MAX_BUF_SIZE 8192

class MyStringBuffer : public std::stringbuf
{
public:
    void seekReadPos(ssize_t nPos)
    {
        std::stringbuf::seekpos(nPos, std::ios_base::in);
    }
    std::string toString()
    {
        std::string temp;
        seekReadPos(0);
        do
        {
            temp.push_back(this->sgetc());
        } while (this->snextc() != EOF);
        seekReadPos(0);
        return temp;
    }
    void seekWritePos(ssize_t nPos)
    {
        std::stringbuf::seekpos(nPos, std::ios_base::out);
    }
    void clear()
    {
        std::stringbuf::setbuf(nullptr, std::streamsize(0));
    }
};

class HashUtils
{
public:
    static int EncodeBase64(const std::string &src, std::string &dest)
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
    static int DecodeBase64(const std::string &src, std::string &dest)
    {
        base64::decoder b64;
        std::stringstream in;
        in << src;
        std::ostringstream out;
        b64.decode(in, out);
        dest = out.str();
        return dest.size();
    }
    static int DecompressWithZlib(MyStringBuffer &in, std::stringstream &out, int WindowsBits)
    {
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        int ret = inflateInit2(&strm, WindowsBits);

        if (ret != Z_OK)
        {
            logger.info("Init infalteInit2 Failed");
            inflateEnd(&strm);
            return -1;
        }
        else
        {
            logger.info("Init infalteInit2 Success");
        }
        char inBuf[MAX_BUF_SIZE], outBuf[MAX_BUF_SIZE];
        int left, size = 0;
        do
        {
            memset(inBuf, 0, MAX_BUF_SIZE);
            size_t nRead = in.sgetn(inBuf, MAX_BUF_SIZE);
            if (nRead < 0)
                break;
            strm.avail_in = nRead;

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

        inflateEnd(&strm);
        return size;
    }

    static int GzipDecompress(MyStringBuffer &in, std::stringstream &out)
    {
        return DecompressWithZlib(in, out, 47);
    }

    static int DeflateDecompress(MyStringBuffer &in, std::stringstream &out)
    {
        return DecompressWithZlib(in, out, -MAX_WBITS);
    }
};
