#include "HttpClient.h"
#include "base/argparse/cmdline.h"

#define AcceptEncoding_Default "gzip, deflate"
#define AcceptLanguage_Default "zh-CN,zh;q=0.9"
#define Accept_Default "*/*"
#define UserAgent_Default "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.183 Safari/537.36"

cmdline::parser cmdParse(int argc, char **argv) {
    cmdline::parser cmdParser;
    cmdParser.add("help", 'h', "This is a simple http client which support http/https");
    cmdParser.add("version", 'v', "show version of client cpp");
    cmdParser.add<string>("url", 'u', "Input the request url", true, "");
    cmdParser.add<string>("type", 't', "requst url type", false, "GET", cmdline::oneof<string>("GET", "POST", "DELETE", "PUT", "HEAD", "Download"));
    cmdParser.add<bool>("enable_redirect", 0, "when response of request's url contains `location` and status code is 3xx, redirect now?", false, false, cmdline::oneof<bool>(true, false));
    cmdParser.add<bool>("verbose", 'V', "display the debug process message", false, false, cmdline::oneof<bool>(true, false));
    cmdParser.add<string>("userAgent", 0, "set request's header userAgent", false, UserAgent_Default);
    cmdParser.add<string>("acceptEncoding", 0, "set request's header AcceptEncoding", false, AcceptEncoding_Default);
    cmdParser.add<string>("acceptLanguage", 0, "set request's header AcceptLanguage", false, AcceptLanguage_Default);
    cmdParser.add<string>("accept", 0, "set request's header Accept Type", false, Accept_Default);
    cmdParser.add<int>("http_version", 0, "set request version type", false, static_cast<int>(HttpVersion::HTTP_1_1),
                       cmdline::oneof<int>(static_cast<int>(HttpVersion::HTTP_1_0), static_cast<int>(HttpVersion::HTTP_1_1)));
    cmdParser.add<string>("cookie", 0, "set request header", false, "");
    cmdParser.add<string>("body", 0, "the post parameter", false, "");
    cmdParser.add<string>("output", 'o', "ouput file path", false, "");
    cmdParser.add<string>("content-type", 0, "the post data content type", false, "application/x-www-form-urlencoded");
    cmdParser.add<int>("logLevel", 'l', "the logger level.(0.debug, 1.info 2.warning 3.alert", false, 1, cmdline::range<int>(0, 3));
    cmdParser.add<int>("timeout", 0, "set connection timeout(s)", false, 10, cmdline::range<int>(0, 30));
    cmdParser.add<string>("auth-basic", 0, "set auth basic user", false, "");
    cmdParser.add<int>("threads", 0, "download threads count", false, 5, cmdline::range<int>(1, 10));
    cmdParser.set_program_name("httpclient");

    return cmdParser;
}

int main(int argc, char *argv[]) {
    auto parser = cmdParse(argc, argv);
    bool ok     = parser.parse(argc, argv);

    if (!ok || argc < 2) {
        std::cout << parser.error() << std::endl;
        std::cout << parser.usage() << std::endl;
        return 0;
    }

    

    return 0;
}
