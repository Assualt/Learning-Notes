#include "tldextract.h"

void Usage() {
    std::cout << "./program [url string]" << std::endl;
    exit(0);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        Usage();
    }

    tld::TLDExtract extract;

    auto Result = extract.extract(argv[ 1 ]);

    logger.info("Extract Url:%s, Url Registed Domain:%s ", argv[ 1 ], Result.RegisterDomain());

    return 0;
}