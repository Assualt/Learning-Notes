#include "DefaultController.h"
#include "base/Logging.h"
#include "net/http/HttpControllMng.h"

REG_CONTROLLER(IController, DefaultController);

DefaultController::DefaultController()
    : IController("Default Controller") {
}

bool DefaultController::onRequest(const HttpRequest &, HttpResponse &, const HttpConfig &) {
    logger.info("Default Controller called");
    return true;
}

void DefaultController::onDump(std::ostream &) {
}

void DefaultController::onInit() {
}