#include "base/argparse/cmdline.h"
#include "tldextract.h"
void Usage() {
    std::cout << "./program [url string]" << std::endl;
    exit(0);
}

int main(int argc, char **argv) {
    auto &log = muduo::base::Logger::getLogger();
    log.BasicConfig(static_cast<LogLevel>(Debug), "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    auto stdHandle = std::make_shared<StdOutLogHandle>();
    log.addLogHandle(stdHandle.get());

    cmdline::parser cmdParse;
    cmdParse.add<std::string>("url", 'u', "input url string", true);
    cmdParse.add<bool>("private_domains", 'p', "extract url use private domains.", false, false, cmdline::oneof<bool>(true, false));
    if (argc < 2) {
        std::cout << cmdParse.usage() << std::endl;
        return 0;
    }

    bool bOk = cmdParse.parse(argc, argv);
    if (!bOk) {
        std::cout << cmdParse.error() << std::endl;
        std::cout << cmdParse.usage() << std::endl;
    } else {
        std::string     urlString         = cmdParse.get<std::string>("url");
        bool            usePrivateDomains = cmdParse.get<bool>("private_domains");
        tld::TLDExtract extract(usePrivateDomains);
        auto            Result = extract.extract(urlString);
        logger.info("Extract Url:%s, Url Register Domain:%s ", urlString, Result.RegisterDomain());
        logger.info("Extract Url:%s, Url Sub Domain:%s ", urlString, Result.SubDomain());
        logger.info("Extract url:%s, Url Domain:%s", urlString, Result.Domain());
    }

    return 0;
}