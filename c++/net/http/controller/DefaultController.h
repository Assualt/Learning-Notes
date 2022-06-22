#include "Controller_if.h"
#include "base/ObjPool.h"
class DefaultController : public IController {
public:
    DefaultController();
    ~DefaultController() override = default;

public:
    bool InitSelf() override;
    bool InitOther() override;
    bool InitFinish() override;
    bool onRequest(const HttpRequest &, HttpResponse &, const HttpConfig &) override;
    void onDump(std::ostream &) override;  
};