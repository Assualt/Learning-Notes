#include "logging.h"

namespace tlog {
std::vector<std::string> detail::levelVec = {"Debug", "Info", "Warning", "Error", "Alert", "Emergency"};

std::map<std::string, int>  detail::levelMap = {{"Debug", 0}, {"Info", 1}, {"Warning", 2}, {"Error", 3}, {"Alert", 4}, {"Emergency", 5}};
std::string    detail::endl     = "\n";
std::string    detail::asctime  = "%04Y-%02m-%02d %02H:%02M:%02S.%03n";
std::string    detail::ctime    = "%02H:%02M:%02S.%06u";
std::set<char> detail::timeOpt  = {
    'Y', /*	Year (1900- now)*/
    'm', /*	Month (0-11) 	*/
    'd', /*	Day of Month (1-31) */
    'H', /* Hour of Day (0-23) */
    'M', /* Minutues of Hour (0-59) */
    'S', /* Seconds of Minitue (0-59) */
    'n', /* Micro Seconds */
    'w', /* the day of Week */
    'Z', /* the Time Zone */
    'T', /* the ISO time string */
    'a', /* the 12 Am. Pm*/
    'u', /* the useconds */
};
std::vector<std::string> detail::WeekDayEngList   = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat"};
std::vector<std::string> detail::MonthEngList     = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
std::set<std::string>    detail::MessageOperation = {"message", "thread", "threadName", "process", "funcName"};
std::set<char>           detail::basicMsgFmt      = {'f', 's', 'd', 'l'};

Logger     Logger::m_Logger;
std::mutex Logger::m_lock;
logImpl    logImpl::LogObject;
} // namespace Log
