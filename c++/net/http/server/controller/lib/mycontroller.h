#include "net/http/server/controller/Controller_if.h"
class MyController : public IController {
public:
    MyController()
        : IController("[my controller]") {}
    virtual ~MyController() override = default;
    DECLARE_CONTROLLER
};