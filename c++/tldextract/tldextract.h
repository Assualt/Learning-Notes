#ifndef _TLD_EXTRACT_H
#define _TLD_EXTRACT_H
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#include "httpclient.hpp"
#include "logging.h"

using namespace std;
#define PUNYCODE "xn--"
#define PUNYPREFIX "//  xn--"
#define ICANNSTART "// ===BEGIN ICANN DOMAINS==="
#define ICANNEND "// ===END ICANN DOMAINS==="

#define USERAGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.183 Safari/537.36"
/**
 * Some useful tools like python tldextract .
 * This Tools achieved most iterfaces such as tldextract.extract()
 * But lack of the function which request data by network.
 * the function which adds some userdefined domain
 * and it only depends on the c++ standard lib.
 */

namespace tld {
struct utils {
public:
    static std::string _ltrim(const std::string &src, char ch = ' ') {
        std::string           temp = src;
        std::string::iterator p    = std::find_if(temp.begin(), temp.end(), [ &ch ](char c) { return ch != c; });
        temp.erase(temp.begin(), p);
        return temp;
    }
    static std::string _rtrim(const std::string &src, char ch = ' ') {
        string                   temp = src;
        string::reverse_iterator p    = find_if(temp.rbegin(), temp.rend(), [ &ch ](char c) { return ch != c; });
        temp.erase(p.base(), temp.end());
        return temp;
    }
    static std::string trim(const std::string &src, char ch = ' ') {
        return utils::_rtrim(utils::_ltrim(src, ch), ch);
    }
    static std::vector<std::string> split(const std::string &src, char divider) {
        std::vector<std::string> result;
        std::string              temp;
        for (auto &ch : src) {
            if (ch == divider) {
                result.push_back(utils::trim(temp));
                temp.clear();
            } else {
                temp.push_back(ch);
            }
        }
        if (!temp.empty())
            result.push_back(utils::trim(temp));
        return result;
    }
    static std::string join(const vector<std::string> &con, char ch, int start = 0, int end = -1, bool skipEmpty = true) {
        std::string result;
        if (end == -1 || (end && end < start))
            end = con.size();
        for (auto i = start; i < end; ++i) {
            if (con[ i ].empty() && skipEmpty)
                continue;
            result.append(con[ i ]);
            if (i != end - 1)
                result.push_back(ch);
        }
        return result;
    }
    static bool startswith(const std::string &src, const std::string &prefix) {
        return src.find(prefix) == 0;
    }
    static bool endswith(const std::string &src, const std::string &prefix) {
        return src.rfind(prefix) == 0;
    }
};
struct ExtractResult {
private:
    std::string m_strSubDomain;
    std::string m_strDomain;
    std::string m_strSuffix;

public:
    ExtractResult(const std::string &subDomain, const std::string &domain, const std::string suffix)
        : m_strSubDomain(subDomain)
        , m_strDomain(domain)
        , m_strSuffix(suffix) {
    }
    std::string SubDomain() const {
        return m_strSubDomain;
    }
    std::string Domain() const {
        return m_strDomain;
    }
    std::string Suffix() const {
        return m_strSuffix;
    }
    std::string RegisterDomain() const {
        if (!m_strDomain.empty() && !m_strSuffix.empty())
            return m_strDomain + "." + m_strSuffix;
        return "";
    }
    std::string Fqdn() const {
        if (!m_strDomain.empty() && !m_strSuffix.empty())
            // self is the namedtuple (subdomain domain suffix)
            return utils::join({m_strSubDomain, m_strDomain, m_strSuffix}, '.');
        return "";
    }
    std::string Ipv4() {
        if (m_strSuffix.empty() && m_strSubDomain.empty() && ExtractResult::checkIpv4(m_strDomain))
            return m_strDomain;
        return "";
    }
    static bool checkIpv4(const std::string &domain) {
        auto partList = utils::split(domain, '.');
        if (partList.size() != 4)
            return false;
        for (auto &part : partList) {
            try {
                auto temp = std::stoi(part.c_str());
                if (temp < 0 || temp > 255)
                    return false;
            } catch (std::exception &e) {
                return false;
            }
        }
        return true;
    }
};
class TLDExtract {
public:
    TLDExtract(const std::string &cache_files = cache_default, const std::string &tld_org_file = "/tmp/.public_suffix_list.dat") {
        initSetting(cache_default, tld_org_file);
    }
    ExtractResult extract(const std::string &url) {
        return extractResult(url);
    }

public:
    static std::set<std::string> suffix_list_urls;
    static std::string           cache_default;
    static size_t                remote_req_timeout;

public:
    bool initOK() const {
        return m_bInitOK;
    }
    void initSetting(const std::string &cache_file, const std::string &tld_org_file) {
        if (load_from_cache_file(cache_file) && !m_sdomainSet.empty())
            return;
        load_org_file(tld_org_file);
        write_down_cache(cache_file);
    }

private:
    void write_down_cache(const std::string &cache_file) {
        std::ofstream fout(cache_file);
        if (!fout.is_open())
            return;
        for (auto &domain : m_sdomainSet)
            fout << domain << ",";
        fout.close();
    }
    bool load_from_cache_file(const std::string &cache_files) {
        if (access(cache_files.c_str(), F_OK) == -1)
            return false;
        std::ifstream fin(cache_files.c_str());
        std::string   result((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        auto          items = utils::split(result, ',');
        for (auto &item : items)
            m_sdomainSet.insert(item);
        fin.close();
        return true;
    }

    bool downloadRemoteUrl(const std::string &destFileName) {
        http::HttpClient client;
        client.setAcceptEncoding("gzip, deflate, br");
        client.setAccept("*/*");
        client.setAcceptLanguage("zh-CN,zh;q=0.9");
        client.setUserAgent(USERAGENT);
        client.setHttpVersion(http::utils::HTTP_1_1);
        for (auto &reqUrl : suffix_list_urls) {
            http::HttpResult result = client.Get(reqUrl);
            if (result.status_code() == 200) {
                client.DownloadToFile(destFileName);
                logger.info("download file %s from %s success.", destFileName, reqUrl);
                return true;
            }
            logger.info("Download %d Error: %s, try another url %s", result.status_code(), result.error(), reqUrl);
        }
        return false;
    }

    bool load_org_file(const std::string &org_file) {
        std::ifstream fin(org_file);
        if (!fin.is_open()) {
            downloadRemoteUrl(org_file);
            fin.open(org_file);
            if (!fin.is_open())
                return false;
        }
        std::string line;
        bool        icann_domain_start = false;
        while (getline(fin, line)) {
            if (line.empty())
                continue;
            else if (line == ICANNSTART) {
                icann_domain_start = true;
                continue;
            } else if (line == ICANNEND) {
                icann_domain_start = false;
                break;
            }
            if (utils::startswith(line, PUNYPREFIX) || utils::startswith(line, "//") && icann_domain_start) {
                std::string punydomain;
                bool        ExistPunydomain = false;
                if (utils::startswith(line, PUNYPREFIX)) {
                    punydomain      = line.substr(3, line.find(' ', 6) - 3);
                    ExistPunydomain = true;
                }
                while (getline(fin, line)) {
                    if (line.empty())
                        break;
                    else if (utils::startswith(line, "//"))
                        continue;
                    m_sdomainSet.insert(utils::trim(line));
                    if (ExistPunydomain)
                        m_mpunyDomainMap[ punydomain ] = utils::trim(line);
                }
            }
        }
        fin.close();
        m_bInitOK = true;
        return true;
    }
    ExtractResult extractResult(const std::string &url) {
        std::string netloc = url;
        if (utils::startswith(netloc, "http://") || utils::startswith(netloc, "https://") || utils::startswith(netloc, "ftp://"))
            netloc = netloc.substr(netloc.find("://") + 3);
        if (netloc.find("/") != std::string::npos)
            netloc = netloc.substr(0, netloc.find("/"));
        if (netloc.find("?") != std::string::npos)
            netloc = netloc.substr(0, netloc.find("?"));
        if (netloc.find("#") != std::string::npos)
            netloc = netloc.substr(0, netloc.find("#"));
        if (netloc.rfind("@") != std::string::npos)
            netloc = netloc.substr(netloc.rfind("@") + 1);
        if (netloc.find(":") != std::string::npos)
            netloc = netloc.substr(0, netloc.find(":"));
        netloc        = utils::trim(netloc);
        netloc        = utils::_rtrim(netloc, '.');
        auto   labels = decode_punycode(utils::split(netloc, '.'));
        size_t index  = suffix_index(labels);

        std::string suffix = utils::join(labels, '.', index);
        if (!suffix.empty() && !netloc.empty() && ExtractResult::checkIpv4(netloc))
            return ExtractResult("", netloc, "");
        std::string subDomain = index ? utils::join(labels, '.', 0, index - 1) : "";
        std::string domain    = index ? labels[ index - 1 ] : "";
        return ExtractResult(subDomain, domain, suffix);
    }
    vector<std::string> decode_punycode(const std::vector<std::string> &labels) {
        vector<std::string> result;
        for (auto label : labels) {
            if (utils::startswith(label, PUNYCODE) && m_mpunyDomainMap.count(label))
                result.push_back(m_mpunyDomainMap[ label ]);
            else
                result.push_back(label);
        }
        return result;
    }
    size_t suffix_index(const std::vector<std::string> &lables) {
        for (size_t i = 0; i < lables.size(); ++i) {
            std::string may_tld       = utils::join(lables, '.', i);
            std::string exception_tld = "!" + may_tld;
            if (m_sdomainSet.count(exception_tld))
                return i + 1;
            if (m_sdomainSet.count(may_tld))
                return i;
            std::string wildcard_tld = "*." + utils::join(lables, '.', i + 1);
            if (m_sdomainSet.count(wildcard_tld))
                return i;
        }
        return lables.size();
    }

private:
    bool                               m_bInitOK;
    std::set<std::string>              m_sdomainSet;
    std::map<std::string, std::string> m_mpunyDomainMap;
};
} // namespace tld
#endif
