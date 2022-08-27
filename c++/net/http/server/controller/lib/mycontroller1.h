#include "net/http/server/controller/Controller_if.h"

class MyController1 : public IController {
public:
    MyController1()
        : IController("[my1 controller]") {}
    virtual ~MyController1() override = default;
    DECLARE_CONTROLLER
};