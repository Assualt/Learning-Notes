#include "httpclient.hpp"

void Usage() {
    std::cout << "./program [url string] [destFile string]" << std::endl;
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        Usage();
    }
    
    http::HttpClient client;
    logger.setLevel(Log::detail::DEBUG);
    // client.setBasicAuthUserPass("xhou", "houXIN19960917");
    client.setConnectTimeout(3);
    // Apache + deflate may be unsuitable for zlib
    client.setAcceptEncoding("gzip, br"); 
    client.setAccept("*/*");
    client.setAcceptLanguage("zh-CN,zh;q=0.9");
    client.setUserAgent("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.183 Safari/537.36");
    client.setHttpVersion(http::utils::HTTP_1_1);
    http::HttpResult Result = client.Get(argv[ 1 ], true, true);

    std::cout << "\nStatus Code:" << Result.status_code() << std::endl
              << "Text Size:" << Result.text().size()   << std::endl
              << "Relay Error:" << Result.error() << std::endl;

    if (argc == 3) {
        client.DownloadToFile(argv[2]);
    }

    return 0;
}