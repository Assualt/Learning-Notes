#include "log.h"
#define Prefix "server.main"
#define Prefix_Trans "server.trans"
int main(int argc, char const *argv[]) {

    tlog::Logger fileLogger(Prefix, "%(asctime)s %(threadname)s %(levelname)s %(message)s", "file.log");
    LogImpl::AppendLogger(Prefix, std::move(fileLogger));
    tlog::Logger transLogger(Prefix_Trans, "%(asctime)s %(threadname)s %(levelname)s %(message)s", "filetrans.log");
    LogImpl::AppendLogger(Prefix_Trans, std::move(transLogger));

    logger.info("Helo World", "123");

    logger.

    return 0;
}
