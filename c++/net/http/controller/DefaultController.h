#include "Controller_if.h"
#include "base/ObjPool.h"

class DefaultController : public IController {
public:
    DefaultController()
        : IController("[Default Controller]", {DefaultPattern, REQ_TYPE::TYPE_GET | REQ_TYPE::TYPE_POST}) {
    }
    virtual ~DefaultController() override = default;
    DECLARE_CONTROLLER
};

class FileIndex : public IController {
public:
    FileIndex()
        : IController("[File Index]", {FilePattern, "GET"}) {
    }
    virtual ~FileIndex() override = default;
    DECLARE_CONTROLLER
};