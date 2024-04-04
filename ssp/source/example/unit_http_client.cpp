//
// Created by 侯鑫 on 2024/1/10.
//

#include "base/cmdline.h"
#include "base/system.h"
#include "net/client/http_client.h"
#include "net/protocol/http_config.h"

using namespace ssp::base;
using namespace ssp::net;

cmdline::parser CreateParser()
{
    cmdline::parser cmdParser;
    cmdParser.add("help", 'h', "This is a simple http client which support http/https");
    cmdParser.add("version", 'v', "show version of client cpp");
    cmdParser.add<std::string>("url", 'u', "Input the request url", true, "");
    cmdParser.add<std::string>("type", 't', "request url type", false, "GET",
                               cmdline::oneof<std::string>("GET", "POST", "DELETE", "PUT", "HEAD", "Download"));
    cmdParser.add<bool>("enable_redirect", 0,
                        "when response of request's url contains `location` and status code is 3xx, redirect now?",
                        false, true, cmdline::oneof<bool>(true, false));
    cmdParser.add<bool>("verbose", 'V', "display the debug process message", false, false,
                        cmdline::oneof<bool>(true, false));
    cmdParser.add<std::string>("userAgent", 0, "set request's header userAgent", false, USER_AGENT_DEFAULT);
    cmdParser.add<std::string>("acceptEncoding", 0, "set request's header AcceptEncoding", false,
                               AcceptEncoding_Default);
    cmdParser.add<std::string>("acceptLanguage", 0, "set request's header AcceptLanguage", false,
                               AcceptLanguage_Default);
    cmdParser.add<std::string>("accept", 0, "set request's header Accept Type", false, Accept_Default);
    cmdParser.add<int>(
        "http_version", 0, "set request version type", false, static_cast<int>(HttpVersion::HTTP_1_1),
        cmdline::oneof<int>(static_cast<int>(HttpVersion::HTTP_1_0), static_cast<int>(HttpVersion::HTTP_1_1)));
    cmdParser.add<std::string>("cookie", 0, "set request header", false, "");
    cmdParser.add<std::string>("body", 0, "the post parameter", false, "");
    cmdParser.add<std::string>("output", 'o', "output file path", false, "");
    cmdParser.add<std::string>("content-type", 0, "the post data content type", false,
                               "application/x-www-form-urlencoded");
    cmdParser.add<int>("logLevel", 'l', "the logger level.(0.debug, 1.info 2.warning 3.alert", false, 2,
                       cmdline::range<int>(0, 3));
    cmdParser.add<int>("timeout", 0, "set connection timeout(s)", false, 10, cmdline::range<int>(0, 30));
    cmdParser.add<std::string>("auth-basic", 0, "set auth basic user", false, "");
    cmdParser.add<int>("threads", 0, "download threads count", false, 5, cmdline::range<int>(1, 10));
    cmdParser.set_program_name("httpclient");

    return cmdParser;
}

void InitHttpRequest(cmdline::parser &parser, HttpClient &client)
{
    auto timeout = parser.get<int>("timeout");
    client.SetTimeOut(timeout, timeout, timeout);
}

int main(int argc, char **argv)
{
    System::SetThreadName("main");
    auto parser = CreateParser();
    if (!parser.parse(argc, argv) || parser.exist("help")) {
        std::cout << parser.error() << std::endl;
        std::cout << parser.usage() << std::endl;
        return 0;
    }

    auto                    &mainLog = Logger::GetLogger();
    std::shared_ptr<LogImpl> _au(new StdoutLog);

    mainLog.BasicConfig(LogLevel::Info, "T:(%(appname)-%(threadName))[%(asctime):%(levelname)]%(message)", "filename",
                        "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());

    HttpClient client;
    client.InitDefaultHeader();
    InitHttpRequest(parser, client);

    auto url      = parser.get<std::string>("url");
    auto reqType  = parser.get<std::string>("type");
    auto redirect = parser.get<bool>("enable_redirect");
    auto verbose  = parser.get<bool>("verbose");

    HttpResponse resp;
    if (reqType == "GET") {
        resp = client.Get(url, redirect, verbose);
    } else if (reqType == "POST") {
        resp = client.Post(url, {}, redirect, verbose);
    } else if (reqType == "HEAD") {
        resp = client.Head(url, redirect, verbose);
    }

    if (!resp.IsOk()) {
        std::cout << resp.Error() << std::endl;
        return 0;
    }

    if (resp.IsTextHtml()) {
        auto [ code, msg ] = resp.GetStatusCode();
        std::cout << "status:" << code << " " << msg << std::endl;
        std::cout << "text:" << std::endl << resp.Text() << std::endl;
    }

    return 0;
}