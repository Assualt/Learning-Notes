#include "hashutils.hpp"
#include <cmdline.hpp>
int main(int argc, char **argv) {
    cmdline::parser CommandParse;

    CommandParse.add<std::string>("type", 't', "The encoding type", false, "base64", cmdline::oneof<std::string>("base64", "gzip", "defalte"));
    CommandParse.add<std::string>("input", 0, "the input string to encode.", true);

    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
    } else {
        std::string type           = CommandParse.get<std::string>("ty  pe");
        std::string strInputString = CommandParse.get<std::string>("input");
        if (strInputString.empty())
            std::cout << "input string is empty. ignored .." << std::endl;
        else {
            std::string resultString;
            if (type == "base64") {
                int ret = HashUtils::EncodeBase64(strInputString, resultString);
                std::cout << "begin to encode base64 source:" << strInputString << std::endl;
                std::cout << "after to encode base64 dest:" << resultString << std::endl;
            }
        }
    }

    return 0;
}