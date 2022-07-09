#include "Controller_if.h"
#include "base/ObjPool.h"

class NotFound : public IController {
public:
    NotFound()
        : IController("notfound", {"/404", "GET"}) {
    }
    virtual ~NotFound() override = default;
    DECLARE_CONTROLLER
};

class MethodFound : public IController {
public:
    MethodFound()
        : IController("methodallow", {"/405", "GET"}) {
    }
    virtual ~MethodFound() override = default;
    DECLARE_CONTROLLER
};

class AuthRequire : public IController {
public:
    AuthRequire()
        : IController("AuthRequire", {"/401", "GET"}) {
    }
    virtual ~AuthRequire() override = default;
    DECLARE_CONTROLLER
};

class BadRequest : public IController {
public:
    BadRequest()
        : IController("badrequest", {"/400", "GET"}) {
    }
    virtual ~BadRequest() override = default;
    DECLARE_CONTROLLER
};