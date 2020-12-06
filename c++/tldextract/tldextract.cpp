#include "tldextract.h"

namespace tld {
std::set<std::string> TLDExtract::suffix_list_urls = {
    "https://publicsuffix.org/list/public_suffix_list.dat",
    "https://raw.githubusercontent.com/publicsuffix/list/master/public_suffix_list.dat",
};
std::string TLDExtract::cache_default      = "/tmp/.tld_cache_set";
size_t      TLDExtract::remote_req_timeout = 10;
} // namespace tld