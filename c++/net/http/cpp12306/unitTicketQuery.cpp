#include "base/Logging.h"
#include "base/StringUtil.h"
#include "base/json/json.h"
#include "net/http/HttpUtils.h"
#include "net/http/client/HttpClient.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
using namespace muduo::net;

namespace tickets {

const std::string stationNameUrl =
    "https://kyfw.12306.cn/otn/resources/js/framework/station_name.js?station_version=1.9167";
const std::string LeftTicketUrl =
    "https://kyfw.12306.cn/otn/leftTicket/"
    "queryZ?leftTicketDTO.train_date=%s&leftTicketDTO.from_station=%s&leftTicketDTO.to_station=%s&purpose_codes=%s";
const std::string MainTicketUrl       = "https://kyfw.12306.cn";
const std::string TicketQueryInitUrl  = "https://kyfw.12306.cn/otn/lcxxcx/init";
const std::string TicketQueryDeviceID = "https://kyfw.12306.cn/otn/HttpZF/logdevice";
const std::string TicketInitUrl       = "https://kyfw.12306.cn/otn/leftTicket/"
                                        "init?linktypeid=dc&fs=%,%&ts=%,%&date=%&flag=%,%,%";

struct StationDetail {
    int         m_nStationNumber;
    std::string m_strChineseName;
    std::string m_strStationCode;
    std::string m_strStationPinYin;
    std::string m_strStationBrief;
    StationDetail(int nStationNumber, const std::string &strChineseName, const std::string &strStationCode,
                  const std::string &strStationPinYin, const std::string &strStationBrief) {
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
        client_.setConnectTimeout(10);
        client_.setHttpVersion("HTTP/1.1");
        client_.setUserAgent("httpclient/1.0");
        client_.setAccept("*/*");
    }

    enum TravelerType { TYPE_CHILD, TYPE_ADULT };
    void initStationDetail(const std::string &strStationPath = "station_name.js");

    void QueryLeftTicket(const std::string &fromStation, const std::string &destStation, const std::string &travelDate,
                         TravelerType type = TYPE_ADULT);
    std::string getStationCode(const std::string &stationName);

    void InitTicketQuery(const std::string &from, const std::string &to, const std::string &date);

private:
    int         SplitStringToStationDetail(const std::string &ss);
    std::string formattedLeftTicketUrl(const std::string &fromStation, const std::string &destStation,
                                       const std::string &travelDate, TravelerType type);
    void        saveStationDetail(const std::string &strStationPath = "station_name.txt");
    void        LoadStationDetail(std::ifstream &fin);

protected:
    std::vector<StationDetail> m_mStationDetailVec;
    HttpClient                 client_;

    std::string m_strDeviceID;        // dfp
    size_t      m_nExpireTimeSeconds; // exp
    std::string m_strRoute;           // route
    std::string m_strJsessionID;      // JSESSIONID
    std::string m_strBIGipServerotn;  // BIGipServerotn
};

void TicketQueryMgr::LoadStationDetail(std::ifstream &fin) {
    fin.seekg(0, ios_base::end);
    off_t len = fin.tellg();
    auto  ptr = std::make_unique<char[]>(len + 1);
    fin.seekg(0, ios_base::beg);
    fin.read(ptr.get(), len);

    std::string err;
    auto        arr = json::Json::parse(ptr.get(), err);
    logger.info("json parse err:%s", err.empty() ? "NO ERR" : err);
    if (arr.is_array()) {
        auto &arrays = arr.array_items();
        std::for_each(arrays.begin(), arrays.end(), [ this ](const json::Json &obj) {
            auto          dict = obj.object_items();
            StationDetail detail;
            detail.m_nStationNumber   = dict[ "stNo" ].int_value();
            detail.m_strChineseName   = dict[ "stCnName" ].string_value();
            detail.m_strStationCode   = dict[ "stCode" ].string_value();
            detail.m_strStationPinYin = dict[ "stPy" ].string_value();
            detail.m_strStationBrief  = dict[ "stBrief" ].string_value();
            m_mStationDetailVec.push_back(std::move(detail));
        });

        logger.info("success load station number:%d", m_mStationDetailVec.size());
    }
}

void TicketQueryMgr::saveStationDetail(const std::string &strStationPath) {
    json::Json allVec(json::Json::ARRAY);
    std::for_each(m_mStationDetailVec.begin(), m_mStationDetailVec.end(), [ &allVec ](const StationDetail &detail) {
        json::Json dict(json::Json::OBJECT);

        dict[ "stNo" ]     = detail.m_nStationNumber;
        dict[ "stCnName" ] = detail.m_strChineseName;
        dict[ "stCode" ]   = detail.m_strStationCode;
        dict[ "stPy" ]     = detail.m_strStationPinYin;
        dict[ "stBrief" ]  = detail.m_strStationBrief;

        allVec.push_back(dict);
    });

    std::ofstream fout(strStationPath);
    fout << allVec.dump();
    fout.close();
}

int TicketQueryMgr::SplitStringToStationDetail(const std::string &strStation) {
    std::string tempString;
    int         nIndex = 0;
    for (size_t i = 0; i < strStation.size(); i++) {
        if (strStation[ i ] == '@') {
            auto valVec = util::splitToVector(tempString, '|');
            if (valVec.size() != 6) {
                logger.info("current %s line is not normal.", tempString);
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
            LoadStationDetail(fin);
            fin.close();
        }
    } else {
        // DownLoad From Here
        logger.info("download station name from %s url.", stationNameUrl);
    download:
        auto response = client_.Get(stationNameUrl, false, true);
        if (response.getStatusCode() != HttpStatusCode::k200Ok) {
            logger.info("Request %s url failed. error:%s", stationNameUrl, response.getStatusMessage());
        } else {
            SplitStringToStationDetail(response.getBody());
            saveStationDetail(strStationPath);
            logger.info("successful to save %s to current path.", strStationPath);
        }
    }
}

void TicketQueryMgr::QueryLeftTicket(const std::string &fromStation, const std::string &destStation,
                                     const std::string &travelDate, TravelerType type) {
    InitTicketQuery(fromStation, destStation, travelDate);
    std::string reqUrl = formattedLeftTicketUrl(fromStation, destStation, travelDate, type);
    logger.info("begin to request url:%s", reqUrl);
    auto result = client_.Get(reqUrl, false, true);
    if (result.getStatusCode() != HttpStatusCode::k200Ok) {
        logger.info("request %s url failed status_code:%d. errmsg:%s", reqUrl, result.getStatusCode(),
                    result.getStatusMessage());
    } else {
        logger.info("request %s url success.", reqUrl);
        logger.info("response Text:%s", result.getBodyBuf().readableBytes());
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

std::string TicketQueryMgr::formattedLeftTicketUrl(const std::string &fromStation, const std::string &destStation,
                                                   const std::string &travelDate, TravelerType type) {
    // "https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date=%s&leftTicketDTO.from_station=%s&leftTicketDTO.to_station=%s&purpose_codes=%s";
    std::string baseString = "https://kyfw.12306.cn/otn/leftTicket/query?";
    baseString.append("leftTicketDTO.train_date=");
    baseString.append(travelDate);
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

void TicketQueryMgr::InitTicketQuery(const std::string &from, const std::string &to, const std::string &date) {
    auto initUrl = FmtString(TicketInitUrl)
                       .arg(UrlUtils::UrlEncode(from))
                       .arg(getStationCode(from))
                       .arg(UrlUtils::UrlEncode(to))
                       .arg(getStationCode(to))
                       .arg(date)
                       .arg("N")
                       .arg("N")
                       .arg("Y")
                       .str();

    logger.info("init url is:%s", initUrl);

    auto resp = client_.Get(initUrl);
    logger.info("get body header cookie %s", resp.getHeader("Set-Cookie"));
}

} // namespace tickets

int main(int, char **) {
    auto &log = muduo::base::Logger::getLogger();
    log.BasicConfig(static_cast<LogLevel>(Info),
                    "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    auto stdHandle = std::make_shared<StdOutLogHandle>();
    log.addLogHandle(stdHandle.get());

    tickets::TicketQueryMgr mgr;
    mgr.initStationDetail();
    logger.info("重庆: Code:%s", mgr.getStationCode("重庆"));
    logger.info("黔江: Code:%s", mgr.getStationCode("黔江"));
    mgr.QueryLeftTicket("天津", "上海", "2022-12-06");

    return 0;
}