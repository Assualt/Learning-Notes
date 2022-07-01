#include "net/http/controller/Controller_if.h"
class MyController : public IController {
public:
    MyController()
        : IController("[my controller]") {
    }
    virtual ~MyController() override = default;
    DECLARE_CONTROLLER
};

extern "C" int mycontroller_Entry(std::string *pattern, int *method, bool *needVal, uintptr_t *);