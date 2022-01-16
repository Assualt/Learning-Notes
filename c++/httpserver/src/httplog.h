#include "httprequest.h"
#include "httpresponse.h"
#include "logging.h"
namespace http {

class HttpLog : public tlog::logImpl {};
} // namespace http