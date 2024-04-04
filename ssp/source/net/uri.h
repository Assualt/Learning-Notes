//
// Created by 侯鑫 on 2024/1/3.
//

#ifndef SSP_TOOLS_URI_H
#define SSP_TOOLS_URI_H

#include <string>
#include <vector>
#include "base/object.h"

namespace ssp::net {

class Uri : public ssp::base::Object {
public:
    Uri() = default;

    /**
     * Parse a Uri from a string.  Throws std::invalid_argument on parse error.
     */
    explicit Uri(std::string_view str);

    [[nodiscard]] const std::string &Scheme() const { return scheme_; }

    [[nodiscard]] const std::string &Username() const { return username_; }

    [[nodiscard]] const std::string &Password() const { return password_; }

    /**
     * Get host part of URI. If host is an IPv6 address, square brackets will be
     * returned, for example: "[::1]".
     */
    [[nodiscard]] const std::string &Host() const { return host_; }

    /**
     * Get host part of URI. If host is an IPv6 address, square brackets will not
     * be returned, for example "::1"; otherwise it returns the same thing as
     * host().
     *
     * hostname() is what one needs to call if passing the host to any other tool
     * or API that connects to that host/port; e.g. getaddrinfo() only understands
     * IPv6 host without square brackets
     */
    [[nodiscard]] std::string Hostname() const;

    uint16_t Port() const { return port_; }

    const std::string &Path() const { return path_; }

    const std::string &Query() const { return query_; }

    const std::string &Fragment() const { return fragment_; }

    [[nodiscard]] std::string Authority() const;

    template <class String> String ToString() const;

    std::string str() const { return ToString<std::string>(); }

    void setPort(uint16_t port)
    {
        hasAuthority_ = true;
        port_         = port;
    }

    /**
     * Get query parameters as key-value pairs.
     * e.g. for URI containing query string:  key1=foo&key2=&key3&=bar&=bar=
     * In returned list, there are 3 entries:
     *     "key1" => "foo"
     *     "key2" => ""
     *     "key3" => ""
     * Parts "=bar" and "=bar=" are ignored, as they are not valid query
     * parameters. "=bar" is missing parameter name, while "=bar=" has more than
     * one equal signs, we don't know which one is the delimiter for key and
     * value.
     *
     * Note, this method is not thread safe, it might update internal state, but
     * only the first call to this method update the state. After the first call
     * is finished, subsequent calls to this method are thread safe.
     *
     * @return  query parameter key-value pairs in a vector, each element is a
     *          pair of which the first element is parameter name and the second
     *          one is parameter value
     */
    const std::vector<std::pair<std::string, std::string>> &GetQueryParams();

    void Dump(std::ostream &os) override;

private:
    std::string                                      scheme_;
    std::string                                      username_;
    std::string                                      password_;
    std::string                                      host_;
    bool                                             hasAuthority_{false};
    uint16_t                                         port_{80};
    std::string                                      path_;
    std::string                                      query_;
    std::string                                      fragment_;
    std::vector<std::pair<std::string, std::string>> queryParams_;
    std::string                                      fullUrl_;
};
}

#endif //SSP_TOOLS_URI_H
