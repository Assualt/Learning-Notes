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
    "queryZ?leftTicketDTO.train_date=%&leftTicketDTO.from_station=%&leftTicketDTO.to_station=%&purpose_codes=%";
const std::string MainTicketUrl       = "https://kyfw.12306.cn";
const std::string TicketQueryInitUrl  = "https://kyfw.12306.cn/otn/lcxxcx/init";
const std::string TicketQueryDeviceID = "https://kyfw.12306.cn/otn/HttpZF/logdevice";
const std::string TicketInitUrl       = "https://kyfw.12306.cn/otn/leftTicket/"
                                        "init?linktypeid=dc&fs=%,%&ts=%,%&date=%&flag=%,%,%";

const std::string queryCookie =
    "_uab_collina=166515664743421659295045; JSESSIONID=3BE2203D333EC5823D12EC28C93E0E05; "
    "RAIL_EXPIRATION=1665447312256; "
    "RAIL_DEVICEID=DwL-OqxMyTV0paSBzDsI4WNYqzjJ0j71tjW2nGCL1Y4TjLXQHEEPxBXr81-AcFRFNh2ON1dlyEowTT-HRRGwH1aWApqDqIVwe_"
    "QkGfbzaVJfzI5_t2CA4vpZSr6j_JQGXrc2ueYsxK1eVw2P8z5AU3_2uJRp1q7n; guidesStatus=off; highContrastMode=defaltMode; "
    "cursorStatus=off; _jc_save_fromStation=%u5929%u6D25%2CTJP; _jc_save_toStation=%u4E0A%u6D77%2CSHH; "
    "_jc_save_wfdc_flag=dc; BIGipServerpassport=1005060362.50215.0000; BIGipServerpool_passport=182714890.50215.0000; "
    "route=9036359bb8a8a461c164a04f8f50b252; _jc_save_toDate=2022-10-08; _jc_save_fromDate=2022-10-08; "
    "BIGipServerotn=552600074.38945.0000; "
    "fo=6m3dym59hdrung68a4qFdNZipa0KnkvOtN00Apip_8QWNjs-peG2Ul82CkwR8-"
    "fY2uRMyU63wQdojKF7ItcsQydjj44t8iFCMR7PEFETtwIgeRPylpvEcyF6dFtqfeZR5hLvx7UaQwI0CyvpnRyvpDol1K7AL44R2PKAFqvjCLmuPYSc"
    "4DAfyyDnlG0RL3nvOMq-HS_dfIRGTtLA";

const std::string queryUserAgent =
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.75 Safari/537.36";

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
        client_.setAcceptEncoding("gzip, deflate");
        client_.setHeader(Connection, "keep-alive");
        client_.setCookie(queryCookie);
        client_.setUserAgent(queryUserAgent);
    }

    enum TravelerType { TYPE_CHILD, TYPE_ADULT };
    void initStationDetail(const std::string &strStationPath = "station_name.js");

    void QueryLeftTicket(const std::string &fromStation, const std::string &destStation, const std::string &travelDate,
                         TravelerType type = TYPE_ADULT);
    std::string getStationCode(const std::string &stationName);

    [[maybe_unused]] void InitTicketQuery(const std::string &from, const std::string &to, const std::string &date);

private:
    int         SplitStringToStationDetail(const std::string &strStation);
    std::string formattedLeftTicketUrl(const std::string &fromStation, const std::string &destStation,
                                       const std::string &travelDate, TravelerType type);
    void        saveStationDetail(const std::string &strStationPath = "station_name.txt");
    void        LoadStationDetail(std::ifstream &fin);

protected:
    std::vector<StationDetail> m_mStationDetailVec;
    HttpClient                 client_;

    std::string m_strDeviceID;          // dfp
    size_t      m_nExpireTimeSeconds{}; // exp
    std::string m_strRoute;             // route
    std::string m_strJsessionID;        // JSESSIONID
    std::string m_strBIGipServerotn;    // BIGipServerotn
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
    for (char i : strStation) {
        if (i == '@') {
            auto valVec = util::splitToVector(tempString, '|');
            if (valVec.size() != 6) {
                logger.info("current %s line is not normal.", tempString);
                continue;
            }
            StationDetail detail(std::atoi(valVec.back().c_str()), valVec[ 1 ], valVec[ 2 ], valVec[ 3 ], valVec[ 4 ]);
            m_mStationDetailVec.push_back(detail);
            nIndex++;
            tempString.clear();
            continue;
        }
        tempString.push_back(i);
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
    for (const auto &item : m_mStationDetailVec) {
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
    std::string travelerType;
    if (type == TYPE_ADULT) {
        travelerType.append("ADUIT");
    } else {
        travelerType.append("CHILD");
    }
    return FmtString(LeftTicketUrl)
        .arg(travelDate)
        .arg(getStationCode(fromStation))
        .arg(getStationCode(destStation))
        .arg(travelerType)
        .str();
}

[[maybe_unused]] void TicketQueryMgr::InitTicketQuery(const std::string &from, const std::string &to,
                                                      const std::string &date) {
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
    log.BasicConfig(static_cast<LogLevel>(Debug),
                    "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    auto stdHandle = std::make_shared<StdOutLogHandle>();
    log.addLogHandle(stdHandle.get());

    tickets::TicketQueryMgr mgr;
    mgr.initStationDetail();
    logger.info("重庆: Code:%s", mgr.getStationCode("重庆"));
    logger.info("黔江: Code:%s", mgr.getStationCode("黔江"));
    mgr.QueryLeftTicket("重庆", "黔江", "2022-10-10");

    return 0;
}