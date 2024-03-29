#include "Uri.h"
#include "base/Conv.h"
#include "base/Format.h"
#include "base/Likely.h"
#include <algorithm>
#include <cctype>
#include <regex>

using namespace muduo::net;
using namespace muduo::base;
namespace {
std::string submatch(const std::cmatch &m, int idx) {
    const auto &sub = m[ idx ];
    return std::string(sub.first, sub.second);
}
} // namespace

Uri::Uri(StringPiece str)
    : hasAuthority_(false)
    , port_(0) {
    static const std::regex uriRegex("([a-zA-Z][a-zA-Z0-9+.-]*):" // scheme:
                                     "([^?#]*)"                   // authority and path
                                     "(?:\\?([^#]*))?"            // ?query
                                     "(?:#(.*))?");               // #fragment
    static const std::regex authorityAndPathRegex("//([^/]*)(/.*)?");

    std::cmatch match;
    if (UNLIKELY(!std::regex_match(str.begin(), str.end(), match, uriRegex))) {
        throw std::invalid_argument(std::string("invalid URI") + str.data());
    }

    scheme_ = submatch(match, 1);
    std::transform(scheme_.begin(), scheme_.end(), scheme_.begin(), ::tolower);

    StringPiece authorityAndPath(match[ 2 ].first, match[ 2 ].second);
    std::cmatch authorityAndPathMatch;
    if (!std::regex_match(authorityAndPath.begin(), authorityAndPath.end(), authorityAndPathMatch,
                          authorityAndPathRegex)) {
        // Does not start with //, doesn't have authority
        hasAuthority_ = false;
        path_         = authorityAndPath.data();
    } else {
        static const std::regex authorityRegex("(?:([^@:]*)(?::([^@]*))?@)?" // username, password
                                               "(\\[[^\\]]*\\]|[^\\[:]*)"    // host (IP-literal (e.g. '['+IPv6+']',
                                                                             // dotted-IPv4, or named host)
                                               "(?::(\\d*))?");              // port

        const auto  authority = authorityAndPathMatch[ 1 ];
        std::cmatch authorityMatch;
        if (!std::regex_match(authority.first, authority.second, authorityMatch, authorityRegex)) {
            throw std::invalid_argument(
                FmtString("invalid URI authority %").arg(StringPiece(authority.first, authority.second).data()).str());
        }

        StringPiece port(authorityMatch[ 4 ].first, authorityMatch[ 4 ].second);
        if (!port.empty()) {
            try {
                port_ = std::stoi(port.data(), nullptr);
            } catch (std::exception &e) {
                throw std::invalid_argument(FmtString("invalid URI port: %").arg(e.what()).str());
            }
        } else if (scheme_ == "https") {
            port_ = 443;
        } else if (scheme_ == "http") {
            port_ = 80;
        } else if (scheme_ == "ftp") {
            port_ = 21;
        } else if (scheme_ == "sftp") {
            port_ = 22;
        }

        hasAuthority_ = true;
        username_     = submatch(authorityMatch, 1);
        password_     = submatch(authorityMatch, 2);
        host_         = submatch(authorityMatch, 3);
        path_         = submatch(authorityAndPathMatch, 2);
    }

    query_    = submatch(match, 3);
    fragment_ = submatch(match, 4);
}

std::string Uri::authority() const {
    std::string result;

    // Port is 5 characters max and we have up to 3 delimiters.
    result.reserve(host().size() + username().size() + password().size() + 8);

    if (!username().empty() || !password().empty()) {
        result.append(username());

        if (!password().empty()) {
            result.push_back(':');
            result.append(password());
        }

        result.push_back('@');
    }

    result.append(host());

    if (port() != 0) {
        result.push_back(':');
        result.append(std::to_string(port()));
    }

    return result;
}

std::string Uri::hostname() const {
    if (!host_.empty() && host_[ 0 ] == '[') {
        // If it starts with '[', then it should end with ']', this is ensured by
        // regex
        return host_.substr(1, host_.size() - 2);
    }
    return host_;
}

const std::vector<std::pair<std::string, std::string>> &Uri::getQueryParams() {
    if (!query_.empty() && queryParams_.empty()) {
        // Parse query string
        static const std::regex queryParamRegex(
            "(^|&)" /*start of query or start of parameter "&"*/
            "([^=&]*)=?" /*parameter name and "=" if value is expected*/
            "([^=&]*)" /*parameter value*/
            "(?=(&|$))" /*forward reference, next should be end of query or
                           start of next parameter*/);
        const std::cregex_iterator paramBeginItr(query_.data(), query_.data() + query_.size(), queryParamRegex);
        std::cregex_iterator       paramEndItr;
        for (auto itr = paramBeginItr; itr != paramEndItr; ++itr) {
            if (itr->length(2) == 0) {
                // key is empty, ignore it
                continue;
            }
            queryParams_.emplace_back(std::string((*itr)[ 2 ].first, (*itr)[ 2 ].second), // parameter name
                                      std::string((*itr)[ 3 ].first, (*itr)[ 3 ].second)  // parameter value
            );
        }
    }
    return queryParams_;
}