#include "hashutils.hpp"
#include "logging.h"
#include <iostream>
#include <sstream>

void Usage() {
    std::cout << "./program [gzipfile string] [destFile string]" << std::endl;
    exit(0);
}

MyStringBuffer stringBuf;
void           process_file(const std::string &filename) {
    if (access(filename.c_str(), F_OK) == -1) {
        logger.info("such %s file name is not exists.", filename);
        return;
    }
    std::ifstream fin(filename, std::ios_base::binary);
    if (!fin.is_open())
        return;
    fin.seekg(0, std::ios_base::end);
    off_t fileLength = fin.tellg();
    char *tempBuffer = new char[ fileLength + 1 ];
    fin.seekg(std::fstream::beg);
    fin.read(tempBuffer, fileLength);
    stringBuf.sputn(tempBuffer, fileLength);
    fin.close();

    // std::stringstream bufferString;
    // size_t bytes = HashUtils::GZipDecompress(tempBuffer, fileLength, bufferString);
    // std::cout << "Decode " << fileLength << " bytes. result bytes is:" << bytes << std::endl;
    delete[] tempBuffer;
    // std::stringstream testBuffer;
    // size_t            bytes = HashUtils::GZipDecompress(stringBuf, testBuffer);
    // std::cout << "Decode " << fileLength << " bytes. result bytes is:" << bytes << std::endl;
}

int uncompress(MyStringBuffer &in, std::stringstream &out) {
    z_stream strm;
    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = 0;
    strm.next_in  = Z_NULL;
    int ret       = inflateInit2(&strm, -MAX_WBITS);

    if (ret != Z_OK) {
        std::cout << "Init infalteInit2 Failed" << std::endl;
        inflateEnd(&strm);
        return -1;
    } else {
        std::cout << "Init infalteInit2 Success" << std::endl;
    }
    char              inBuf[ MAX_BUF_SIZE ], outBuf[ MAX_BUF_SIZE ];
    int               left, size = 0;
    do {
        memset(inBuf, 0, MAX_BUF_SIZE);
        size_t nRead = stringBuf.sgetn(inBuf, MAX_BUF_SIZE);
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

int main(int argc, char **argv) {
    if (argc < 3) {
        Usage();
    }
    process_file(argv[ 1 ]);
    std::stringstream out;
    int               size = uncompress(stringBuf, out);
	logger.info("Decode buffer size:%d", size);

    std::ofstream fout(argv[2], std::ios_base::binary);
    if(!fout.is_open()){
        logger.info("open %s file error.", argv[2]);
        return 0;
    }
    fout.write(out.str().c_str(), size);
    fout.close();
    logger.info("end to save %s file.", argv[2]);

    return 0;
}
