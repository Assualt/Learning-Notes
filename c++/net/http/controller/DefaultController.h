#include "Controller_if.h"
class DefaultController : public IController {
public:
    DefaultController();
    ~DefaultController() override = default;

public:
    bool onRequest(const HttpRequest &, HttpResponse &, const HttpConfig &) override;
    void onDump(std::ostream &) override;
    void onInit() override;
};