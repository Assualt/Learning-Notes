//
// Created by 侯鑫 on 2024/1/3.
//

#include "uri.h"
#include "base/format.h"
#include "base/likely.h"
#include <algorithm>
#include <cctype>
#include <regex>

using namespace ssp::net;
using namespace ssp::base;
namespace {
std::string submatch(const std::cmatch &m, int idx)
{
    const auto &sub = m[ idx ];
    return {sub.first, sub.second};
}
} // namespace

Uri::Uri(std::string_view str)
        : hasAuthority_(false)
        , port_(0)
        , fullUrl_(str.data())
{
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

    std::string_view authorityAndPath(match[2].first, match[2].second - match[2].first);
    std::cmatch authorityAndPathMatch;
    if (!std::regex_match(authorityAndPath.begin(), authorityAndPath.end(), authorityAndPathMatch,
                          authorityAndPathRegex)) {
        // Does not start with //, doesn't have authority
        hasAuthority_ = false;
        path_         = authorityAndPath.data();
    } else {
        static const std::regex authorityRegex("(?:([^@:]*)(?::([^@]*))?@)?" // username, password
                                               "(\\[[^\\]]*\\]|[^\\[:]*)"    // host (IP-literal (e.g. '['+IPv6+']',
                                                                             // dotted-IPv4, or named host
                                               "(?::(\\d*))?");              // port

        const auto  authority = authorityAndPathMatch[ 1 ];
        std::cmatch authorityMatch;
        if (!std::regex_match(authority.first, authority.second, authorityMatch, authorityRegex)) {
            throw std::invalid_argument(
                    FmtString("invalid URI authority %").arg(std::string(authority.first, authority.second).data()).str());
        }

        std::string port(authorityMatch[ 4 ].first, authorityMatch[ 4 ].second);
        if (!port.empty()) {
            try {
                port_ = std::stoi(port, nullptr);
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

std::string Uri::Authority() const
{
    std::string result;

    // Port is 5 characters max and we have up to 3 delimiters.
    result.reserve(Host().size() + Username().size() + Password().size() + 8);

    if (!Username().empty() || !Password().empty()) {
        result.append(Username());

        if (!Password().empty()) {
            result.push_back(':');
            result.append(Password());
        }

        result.push_back('@');
    }

    result.append(Host());
    if (Port() != 0) {
        result.push_back(':');
        result.append(std::to_string(Port()));
    }

    return result;
}

std::string Uri::Hostname() const
{
    if (!host_.empty() && host_[ 0 ] == '[') {
        // If it starts with '[', then it should end with ']', this is ensured by
        // regex
        return host_.substr(1, host_.size() - 2);
    }
    return host_;
}

const std::vector<std::pair<std::string, std::string>> &Uri::GetQueryParams()
{
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
            if (itr->empty()) {
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

void Uri::Dump(std::ostream &os)
{
    os << ">>> Uri:" << "\n"
       << "  ful    :" << fullUrl_ << "\n"
       << "  scheme :" << scheme_ << "\n"
       << "  user   :" << username_ << "\n"
       << "  pass   :" << password_ << "\n"
       << "  host   :" << host_ << "\n"
       << "  port   :" << port_ << "\n"
       << "  path   :" << path_ << "\n"
       << "  query  :" << query_ << "\n"
       << "  frags  :" << fragment_ << "\n";
}