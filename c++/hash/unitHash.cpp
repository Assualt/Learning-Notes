#include "hashutils.hpp"
#include <cmdline.hpp>
#include <fstream>
std::string loadFileString(const std::string &filePath) {
    if (access(filePath.c_str(), F_OK) == -1) {
        return "";
    }
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp == nullptr)
        return "";
    std::stringstream ss;
    char              ch;
    while ((ch = fgetc(fp)) != EOF) {
        ss << ch;
    }
    fclose(fp);
    return ss.str();
}
int writeFile(const std::string &destFile, MyStringBuffer &buffer) {
    FILE *fp = fopen(destFile.c_str(), "wb");
    if (fp == nullptr)
        return 0;
    buffer.seekReadPos(0);
    size_t nBuf = 0;

    do {
        fputc(buffer.sgetc(), fp);
        nBuf++;
    } while (buffer.snextc() != EOF);
    fclose(fp);
    return nBuf;
}

int main(int argc, char **argv) {
    cmdline::parser CommandParse;

    CommandParse.add<std::string>("type", 't', "The encoding type", false, "base64", cmdline::oneof<std::string>("base64", "gzip", "deflate", "zlib"));
    CommandParse.add<std::string>("input", 0, "the input string to encode.", true);
    CommandParse.add<std::string>("output", 'o', "the output file.", false, "result.txt");

    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
    } else {
        std::string type           = CommandParse.get<std::string>("type");
        std::string strInputString = CommandParse.get<std::string>("input");
        std::string strOutputFile  = CommandParse.get<std::string>("output");
        if (strInputString.empty())
            std::cout << "input string is empty. ignored .." << std::endl;
        else {
            std::string resultString;
            if (type == "base64") {
                int ret = HashUtils::EncodeBase64(strInputString, resultString);
                std::cout << "begin to encode base64 source:" << strInputString << std::endl;
                std::cout << "after to encode base64 dest:" << resultString << std::endl;
            } else if (type == "gzip") {
                std::string    fileString = loadFileString(strInputString);
                MyStringBuffer buffer;
                auto           size = HashUtils::GzipCompress((void *)fileString.c_str(), fileString.size(), buffer);
                std::cout << "Compress: " << fileString.size() << " bytes. after compressed :" << size << std::endl;
                auto writeSize = writeFile(strOutputFile, buffer);
                std::cout << "write File" << strOutputFile << " Size:" << writeSize << std::endl;
                std::stringstream out;
                buffer.seekReadPos(0);
                auto DecodeSize = HashUtils::GzipDecompress(buffer, out);
                std::cout << "DeCompress: " << writeSize << " bytes. after Decompressed :" << DecodeSize << std::endl;
                std::cout << "Decode String:\n" << out.str() << std::endl;
            } else if (type == "deflate") {
                std::string    fileString = loadFileString(strInputString);
                MyStringBuffer buffer;
                auto           size = HashUtils::DeflateCompress((void *)fileString.c_str(), fileString.size(), buffer);
                std::cout << "Compress: " << fileString.size() << " bytes. after compressed :" << size << std::endl;
                auto writeSize = writeFile(strOutputFile, buffer);
                std::cout << "write File" << strOutputFile << " Size:" << writeSize << std::endl;
                std::stringstream out;
                buffer.seekReadPos(0);
                auto DecodeSize = HashUtils::DeflateDecompress(buffer, out);
                std::cout << "DeCompress: " << writeSize << " bytes. after Decompressed :" << DecodeSize << std::endl;
                std::cout << "Decode String:\n" << out.str() << std::endl;
            } else if (type == "zlib") {
                std::string    fileString = loadFileString(strInputString);
                MyStringBuffer buffer;
                auto           size = HashUtils::ZlibCompress((void *)fileString.c_str(), fileString.size(), buffer);
                std::cout << "Compress: " << fileString.size() << " bytes. after compressed :" << size << std::endl;
                auto writeSize = writeFile(strOutputFile, buffer);
                std::cout << "write File" << strOutputFile << " Size:" << writeSize << std::endl;
                std::stringstream out;
                buffer.seekReadPos(0);
                auto DecodeSize = HashUtils::ZlibDeCompress(buffer, out);
                std::cout << "DeCompress: " << writeSize << " bytes. after Decompressed :" << DecodeSize << std::endl;
                std::cout << "Decode String:\n" << out.str() << std::endl;
            }
        }
    }

    return 0;
}