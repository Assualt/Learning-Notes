#include "cmdline.hpp"
#include "tldextract.h"
void Usage() {
    std::cout << "./program [url string]" << std::endl;
    exit(0);
}

int main(int argc, char **argv) {

    cmdline::parser cmdparse;
    cmdparse.add<std::string>("url", 'u', "input url string", true);
    if (argc < 2) {
        std::cout << cmdparse.usage() << std::endl;
        return 0;
    }

    bool bOk = cmdparse.parse(argc, argv);
    if (!bOk) {
        std::cout << cmdparse.usage() << std::endl;
    } else {
        std::string     urlString = cmdparse.get<std::string>("url");
        tld::TLDExtract extract;
        auto            Result = extract.extract(urlString);
        logger.info("Extract Url:%s, Url Registed Domain:%s ", urlString, Result.RegisterDomain());
        logger.info("Extract Url:%s, Url Sub Domain:%s ", urlString, Result.SubDomain());
        logger.info("Extract url:%s, Url Domain:%s", urlString, Result.Domain());
    }

    return 0;
}