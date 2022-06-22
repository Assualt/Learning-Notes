#include "DefaultController.h"
#include "base/Logging.h"

REG_OBJECT(DefaultController)
DefaultController::DefaultController()
    : IController("Default Controller") {
}

bool DefaultController::onRequest(const HttpRequest &, HttpResponse &, const HttpConfig &) {
    logger.info("Default Controller called");
    return true;
}

void DefaultController::onDump(std::ostream &) {
}

bool DefaultController::InitSelf() {
    return true;
}

bool DefaultController::InitOther() {
    return true;
}

bool DefaultController::InitFinish() {
    return true;
}
