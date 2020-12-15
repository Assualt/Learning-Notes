#include "logging.h"
#include "httprequest.h"
#include "httpresponse.h"
namespace http {

class HttpLog: public tlog::logImpl {
};
} // namespace http