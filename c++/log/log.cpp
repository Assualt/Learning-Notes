//
// Created by Sunshine on 2020/8/30.
//
#include "log.h"

namespace tlog {
Logger                           Logger::stdOutLogger;
std::map<std::string, Logger &&> logImpl::g_LoggerMap;
std::map<std::string, int>       detail::levelMap          = {{"Debug", 0}, // NOLINT
                                               {"Info", 1},  {"Warning", 2}, {"Error", 3}, {"Alert", 4}, {"Emergency", 5}};
std::string                      detail::ctime             = "%02H:%02M:%02S.%06u";
std::string                      detail::timeFmt           = "%04Y-%02m-%02d %02H:%02M:%02S.%03n"; /* NO INT */
std::vector<std::string>         detail::levelStringVector = {                                     // NOINT
    "Debug",    "Info", "Warning", "Errors",
    "Alert",      // NOLINT
    "Emergency"}; // NOLINT

std::vector<std::string> detail::WeekDayEngList = {"Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat"}; // NOLINT
std::vector<std::string> detail::MonthEngList   = {"Jan",  "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                                                 "Aug",                        // NOLINT
                                                 "Sept", "Oct", "Nov", "Dec"}; // NOLINT
std::set<char>           detail::basicMsgFmt    = {'f', 's', 'd', 'l'};          // NOLINT
std::set<char>           detail::timeOperator   = {
    // NOLINT
    'Y', /*	Year (1900- now)*/
    'm', /*	Month (0-11) 	*/
    'd', /*	Day of Month (1-31) */
    'H', /* Hour of Day (0-23) */
    'M', /* Minutes of Hour (0-59) */
    'S', /* Seconds of Minute (0-59) */
    'n', /* Micro Seconds */
    'w', /* the day of Week */
    'Z', /* the Time Zone */
    'T', /* the ISO time string */
    'a', /* the 12 Am. Pm*/
    'u', /* the useconds */
};
} // namespace tlog