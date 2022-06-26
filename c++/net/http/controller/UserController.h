#include "Controller_if.h"
#include "base/ObjPool.h"

class UserController : public IController {
public:
    UserController()
        : IController("[dir Controller]", {"/login/{user}/{id}", REQ_TYPE::TYPE_POST, true}) {
    }
    virtual ~UserController() override = default;
    DECLARE_CONTROLLER
};