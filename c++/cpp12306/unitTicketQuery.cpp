#include "httpclient.hpp"
#include "json.hpp"
#include "logging.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
using namespace std;

namespace tickets {

static std::string _ltrim(const std::string &src, char ch = ' ') {
    std::string           temp = src;
    std::string::iterator p    = std::find_if(temp.begin(), temp.end(), [&ch](char c) { return ch != c; });
    temp.erase(temp.begin(), p);
    return temp;
}
static std::string _rtrim(const std::string &src, char ch = ' ') {
    string                   temp = src;
    string::reverse_iterator p    = find_if(temp.rbegin(), temp.rend(), [&ch](char c) { return ch != c; });
    temp.erase(p.base(), temp.end());
    return temp;
}
static std::string trim(const std::string &src, char ch = ' ') {
    return _rtrim(_ltrim(src, ch), ch);
}

static std::vector<std::string> splitToVector(const std::string &strVal, char ch = ' ', bool skipEmpty = true) {
    std::vector<std::string> result;
    std::string              temp;
    for (auto c : strVal) {
        if (c == ch) {
            temp = trim(temp);
            if (temp.empty() && skipEmpty)
                continue;
            result.push_back(temp);
            temp.clear();
        } else {
            temp.push_back(c);
        }
    }
    temp = trim(temp);
    if (!temp.empty())
        result.push_back(temp);
    else if (temp.empty() && !skipEmpty)
        result.push_back(temp);
    return result;
}
static std::string replace(const std::string &strVal, char src, char dest) {
    std::string result;
    for (auto i = 0; i < strVal.size(); i++) {
        if (strVal[ i ] == src) {
            result.push_back(dest);
        } else {
            result.push_back(strVal[ i ]);
        }
    }
    return result;
}

const std::string StationNameUrl      = "https://kyfw.12306.cn/otn/resources/js/framework/station_name.js?station_version=1.9167";
const std::string LeftTicketUrl       = "https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date=%s&leftTicketDTO.from_station=%s&leftTicketDTO.to_station=%s&purpose_codes=%s";
const std::string MainTicketUrl       = "https://kyfw.12306.cn";
const std::string TicketQueryInitUrl  = "https://kyfw.12306.cn/otn/lcxxcx/init";
const std::string TicketQueryDeviceID = "https://kyfw.12306.cn/otn/HttpZF/logdevice";

struct StationDetail {
    int         m_nStationNumber;
    std::string m_strChineseName;
    std::string m_strStationCode;
    std::string m_strStationPinYin;
    std::string m_strStationBrief;
    StationDetail(int nStationNumber, const std::string &strChineseName, const std::string &strStationCode, const std::string &strStationPinYin, const std::string &strStationBrief) {
        m_nStationNumber   = nStationNumber;
        m_strChineseName   = strChineseName;
        m_strStationCode   = strStationCode;
        m_strStationPinYin = strStationPinYin;
        m_strStationBrief  = strStationBrief;
    }
    StationDetail() = default;

    friend ostream &operator<<(ostream &os, StationDetail &detail) {
        os << "No:" << detail.m_nStationNumber << " "
           << "Code:" << detail.m_strStationCode << " "
           << "Chinese Name:" << detail.m_strChineseName << " "
           << "PinYin:" << detail.m_strStationPinYin << " "
           << "Brief:" << detail.m_strStationBrief;
        return os;
    }
};

class TicketQueryMgr {
public:
    TicketQueryMgr() {
        m_HttpClient.setConnectTimeout(3);
        m_HttpClient.setAcceptEncoding("gzip, deflate, br");
        m_HttpClient.setAccept("*/*");
        m_HttpClient.setAcceptLanguage("zh-CN,zh;q=0.9");
        m_HttpClient.setUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0");
        m_HttpClient.setHttpVersion(http::utils::HTTP_1_1);
        m_HttpClient.setHeader("Connection", "keep-alive");
    }

    void setCookie(const std::string &strCookie) {
        m_HttpClient.setCookie(strCookie);
    }

    void initCookie(const std::string &strDeviceNamePath) {
        if (access(strDeviceNamePath.c_str(), F_OK) != -1) {
            std::ifstream fin(strDeviceNamePath.c_str());
            if (!fin.is_open()) {
                logger.info("open %s file for get cookie and device id failed.", strDeviceNamePath);
            } else {
                std::string temp;
                while (std::getline(fin, temp)) {
                    size_t nPos;
                    if (!temp.empty() && (nPos = temp.find("=")) != std::string::npos) {
                        std::string strKey = temp.substr(0, nPos);
                        if (strcasecmp("exp", strKey.c_str()) == 0) {
                            m_nExpairTimeSeconds = atol(temp.substr(nPos + 1).c_str());
                        } else if (strcasecmp("dfp", strKey.c_str()) == 0) {
                            m_strDeviceID = temp.substr(nPos + 1);
                        } else if (strcasecmp("route", strKey.c_str()) == 0) {
                            m_strRoute = temp.substr(nPos + 1);
                        } else if (strcasecmp("jsessionid", strKey.c_str()) == 0) {
                            m_strJsessionID = temp.substr(nPos + 1);
                        } else if (strcasecmp("bigipserverotn", strKey.c_str()) == 0) {
                            m_strBIGipServerotn = temp.substr(nPos + 1);
                        }
                    }
                }
            }
            logger.info("route:%s, jsessionid:%s, bigipserverotn:%s,exp:%s, fdp:%s", m_strRoute, m_strJsessionID, m_strBIGipServerotn, m_nExpairTimeSeconds, m_strDeviceID);
        } else {
            auto        Response  = m_HttpClient.Get(TicketQueryInitUrl, false, true);
            std::string strCookie = m_HttpClient.getCookie();
            logger.info("get cookie from remote url:%s, cookie:%s", strCookie);
            if (Response.status_code() == 200) {
                if (!strCookie.empty()) {
                    auto ItemVec = splitToVector(strCookie, ';');
                    for (auto &item : ItemVec) {
                        std::string strKey = item.substr(0, item.find("="));
                        std::string strVal = item.substr(item.find("=") + 1);
                        if (strKey == "route")
                            m_strRoute = strVal;
                        else if (strKey == "JSESSIONID")
                            m_strJsessionID = strVal;
                        else if (strKey == "BIGipServerotn")
                            m_strBIGipServerotn = strVal;
                    }
                }
            } else {
                logger.info("request %s url failed. status_code:%d, status message:%s", TicketQueryDeviceID, Response.status_code(), Response.error());
            }
            logger.info("route:%s, jsessionid:%s, bigipserverotn:%s", m_strRoute, m_strJsessionID, m_strBIGipServerotn);
            // get exp, and dfp
            logger.info("begin to request %s", TicketQueryDeviceID);
            Response = m_HttpClient.Get(TicketQueryDeviceID, false, true);
            if (Response.status_code() == 200) {
                std::string text = Response.text();
                if (text.find("callbackFunction") == 0) {
                    text = text.substr(text.find("{"), text.rfind("}") - text.find("{") + 1);
                    logger.info("get response text:%s", text);
                    std::string strErrString;
                    json::Json  tempJson = json::Json::parse(text, strErrString);
                    m_nExpairTimeSeconds = atol(tempJson[ "exp" ].string_value().c_str());
                    m_strDeviceID        = tempJson[ "dfp" ].string_value();
                } else {
                    logger.info("get dfp error. %s", text);
                }
            } else {
                logger.warning("request %s url failed. status_code:%d, status message:%s", TicketQueryDeviceID, Response.status_code(), Response.error());
            }
            logger.info("route:%s, jsessionid:%s, bigipserverotn:%s", m_strRoute, m_strJsessionID, m_strBIGipServerotn);
            std::ofstream fout(strDeviceNamePath.c_str());
            fout << "route=" << m_strRoute << std::endl;
            fout << "JSESSIONID=" << m_strJsessionID << std::endl;
            fout << "BIGipServerotn=" << m_strBIGipServerotn << std::endl;
            fout << "exp=" << m_nExpairTimeSeconds << std::endl;
            fout << "dfp=" << m_strDeviceID << std::endl;
            fout.close();
        }
        // get Cookie
        std::string strCookie;
        strCookie.append("JSESSIONID=");
        strCookie.append(m_strJsessionID);
        strCookie.append("; BIGipServerotn=");
        strCookie.append(m_strBIGipServerotn);
        strCookie.append("; RAIL_EXPIRATION=");
        strCookie.append(std::to_string(m_nExpairTimeSeconds));
        strCookie.append("; RAIL_DEVICEID=");
        strCookie.append(m_strDeviceID);
        strCookie.append("; route=");
        strCookie.append(m_strRoute);
        m_HttpClient.setCookie(strCookie);
        logger.info("set Cookie:%s", strCookie);
    }

    enum TravalerType { TYPE_CHILD, TYPE_ADULT };
    void        initStationDetail(const std::string &strStationPath = "station_name.js");
    void        QueryLeftTicket(const std::string &fromStation, const std::string &destStation, const std::string &Traveldate, TravalerType type = TYPE_ADULT);
    std::string getStationCode(const std::string &stationName);

private:
    int         SplitStringToStationDetail(const std::string &ss);
    std::string formattedLeftTicketUrl(const std::string &fromStation, const std::string &destStation, const std::string &Traveldate, TravalerType type);

protected:
    std::vector<StationDetail> m_mStationDetailVec;
    http::HttpClient           m_HttpClient;

    std::string m_strDeviceID;        // dfp
    size_t      m_nExpairTimeSeconds; // exp
    std::string m_strRoute;           // route
    std::string m_strJsessionID;      // JSESSIONID
    std::string m_strBIGipServerotn;  // BIGipServerotn
};

int TicketQueryMgr::SplitStringToStationDetail(const std::string &strStation) {
    std::string tempString;
    int         nIndex = 0;
    for (size_t i = 0; i < strStation.size(); i++) {
        if (strStation[ i ] == '@') {
            auto valVec = splitToVector(tempString, '|');
            if (valVec.size() != 6) {
                logger.info("current %s line is not nomral.", tempString);
                continue;
            }
            StationDetail detail(atoi(valVec.back().c_str()), valVec[ 1 ], valVec[ 2 ], valVec[ 3 ], valVec[ 4 ]);
            m_mStationDetailVec.push_back(detail);
            nIndex++;
            tempString.clear();
            continue;
        }
        tempString.push_back(strStation[ i ]);
    }
    logger.info("success insert %d station into map. totalSize:%d", nIndex, m_mStationDetailVec.size());
    return nIndex;
}

void TicketQueryMgr::initStationDetail(const std::string &strStationPath) {
    if (access(strStationPath.c_str(), F_OK) != -1) {
        logger.info("begin to load station detail from %s file.", strStationPath);
        std::ifstream fin(strStationPath);
        if (!fin.is_open()) {
            logger.info("open %s file failed. and download station detail from url now.");
            goto download;
        } else {
            fin.seekg(0, std::ios_base::end);
            off_t nFileSize = fin.tellg();
            fin.seekg(0, std::ios_base::beg);
            char *tempString = new char[ nFileSize + 1 ];
            fin.read(tempString, nFileSize);
            std::string ss(tempString, nFileSize);
            SplitStringToStationDetail(ss);
            delete[] tempString;
        }
    } else {
        // DownLoad From Here
        logger.info("download station name from %s url.", StationNameUrl);
    download:
        http::HttpResult response = m_HttpClient.Get(StationNameUrl, false, false);
        if (response.status_code() != 200) {
            logger.info("Request %s url failed. error:%s", StationNameUrl, response.error());
        } else {
            SplitStringToStationDetail(response.text());
            m_HttpClient.SaveResultToFile(strStationPath);
            logger.info("successful to save %s to current path.", strStationPath);
        }
    }
}

void TicketQueryMgr::QueryLeftTicket(const std::string &fromStation, const std::string &destStation, const std::string &Traveldate, TravalerType type) {
    std::string requstUrl = formattedLeftTicketUrl(fromStation, destStation, Traveldate, type);
    logger.info("begin to request url:%s", requstUrl);
    http::HttpResult result = m_HttpClient.Get(requstUrl, false, true);
    if (result.status_code() != 200) {
        logger.info("request %s url failed status_code:%d. errmsg:%s", requstUrl, result.status_code(), result.error());
    } else {
        logger.info("requst %s url success.", requstUrl);
        logger.info("response Text:%s", result.text());
    }
}

std::string TicketQueryMgr::getStationCode(const std::string &stationName) {
    for (auto item : m_mStationDetailVec) {
        if (item.m_strStationPinYin == stationName) {
            return item.m_strStationCode;
        }
        if (item.m_strChineseName == stationName) {
            return item.m_strStationCode;
        }
        if (item.m_strStationBrief == stationName) {
            return item.m_strStationCode;
        }
    }
    return "";
}

std::string TicketQueryMgr::formattedLeftTicketUrl(const std::string &fromStation, const std::string &destStation, const std::string &Traveldate, TravalerType type) {
    // "https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date=%s&leftTicketDTO.from_station=%s&leftTicketDTO.to_station=%s&purpose_codes=%s";
    std::string baseString = "https://kyfw.12306.cn/otn/leftTicket/query?";
    baseString.append("leftTicketDTO.train_date=");
    baseString.append(Traveldate);
    baseString.append("&");
    baseString.append("leftTicketDTO.from_station=");
    baseString.append(getStationCode(fromStation));
    baseString.append("&");
    baseString.append("leftTicketDTO.to_station=");
    baseString.append(getStationCode(destStation));
    baseString.append("&");
    baseString.append("purpose_codes=");
    if (type == TYPE_ADULT) {
        baseString.append("ADUIT");
    } else {
        baseString.append("CHILD");
    }
    return baseString;
}

} // namespace tickets

int main(int argc, char **argv) {

    tickets::TicketQueryMgr mgr;
    mgr.initStationDetail();
    logger.info("重庆: Code:%s", mgr.getStationCode("重庆"));
    logger.info("黔江: Code:%s", mgr.getStationCode("黔江"));
    mgr.initCookie("device.txt");
    // mgr.setCookie("_uab_collina=160656657635693108674672; JSESSIONID=891DA34F47EEF0717893CC950E8CB909; _jc_save_fromStation=%u5929%u6D25%2CTJP; _jc_save_toStation=%u4E0A%u6D77%2CSHH;
    // _jc_save_wfdc_flag=dc; RAIL_EXPIRATION=1607489144437;
    // RAIL_DEVICEID=d_mD28yaHf0FKmb-bVUbLO5AF6qKEqTx9CYV_Hi01SF6tnsApcW8PBludi7HDrLtx45gAK9jBny3qrfmvveansnIDbvxn75bPCPDRxm42McvQjG4S1SvfxpKuxaZrK-VovMVDuHzXAJJPQmWVnFetMx5Iv2XxXSZ;
    // BIGipServerpassport=1005060362.50215.0000; route=6f50b51faa11b987e576cdb301e545c4; BIGipServerotn=2263351562.24610.0000; _jc_save_fromDate=2020-12-06; _jc_save_toDate=2020-12-06");
    mgr.QueryLeftTicket("天津", "上海", "2020-12-06");

    return 0;
}