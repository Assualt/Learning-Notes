#include "base/cmdline.h"
#include "base/log_impl.h"
#include "tldextract.h"

using namespace ssp::base;
using namespace ssp::net;

void Usage() {
    std::cout << "./program [url string]" << std::endl;
    exit(0);
}

int main(int argc, char **argv) {
    auto &log = Logger::GetLogger();
    log.BasicConfig(static_cast<LogLevel>(LogLevel::Debug),
                    "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.SetAppName("app");
    auto stdHandle = std::make_shared<StdoutLog>();
    log.AddLogHandle(stdHandle.get());

    cmdline::parser cmdParse;
    cmdParse.add<std::string>("url", 'u', "input url string", true);
    cmdParse.add<bool>("private_domains", 'p', "extract url use private domains.", false, false,
                       cmdline::oneof<bool>(true, false));
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
        logger.Info("Extract Url:%s, Url Register Domain:%s ", urlString, Result.RegisterDomain());
        logger.Info("Extract Url:%s, Url Sub Domain:%s ", urlString, Result.SubDomain());
        logger.Info("Extract url:%s, Url Domain:%s", urlString, Result.Domain());
    }

    return 0;
}