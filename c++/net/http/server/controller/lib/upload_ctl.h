#pragma once
#include "net/http/server/controller/Controller_if.h"

class UploadController : public IController {
public:
    UploadController()
        : IController("[upload controller]") {
    }
    virtual ~UploadController() override = default;
    DECLARE_CONTROLLER
};