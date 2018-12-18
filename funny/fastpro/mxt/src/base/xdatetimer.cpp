#include "base/xdatetimer.h"

NAMESPACE_BEGIN

//static init

char TDateTimer::m_strMon[12][10] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
char TDateTimer::m_strWeek[7][10] = { "Sun", "Mon", "Tue", "Wed","Thu", "Fri", "Sat" };
char TDateTimer::m_strMonF[12][20] = { "January", "February", "March", "April", "May", "June","July", "August", "September", "October", "November", "December" };
char TDateTimer::m_strWeekF[7][20] = { "Sunday", "Monday", "Tuesday", "Wednesday","Thursday", "Friday", "Saturday" };

bool TDateTimer::isValidYear(int nYear, int nMon, int nDay)
{
	bool bleap = TDateTimer::isLeapYear(nYear);
	if (nMon >= 1 && nMon <= 12)
	{
		if (nMon == 2)
			return nDay <= (bleap ? 29 : 28) && nDay >= 1;
		else if (nMon == 4 || nMon == 6 || nMon == 9 || nMon == 11)
			return nDay <= 30 && nDay >= 1;
		else
			return nDay <= 31 && nDay >= 1;
	}
	return false;
}
bool TDateTimer::isValidTime(int nHour, int nMin, int nSec, TYPE_TIME AmPm)
{
	if (AmPm == TDateTimer::TYPE_12_PM)
		nHour += 12;
	return (nHour >= 0 && nHour <= 23) && (nMin >= 0 && nMin <= 59) && (nSec >= 0 && nSec <= 59);
}
bool TDateTimer::isLeapYear(int nYear)
{
	if (nYear < 1752)
		return ((nYear & 3) == 0);
	else
		return (((nYear & 3) == 0 && (nYear % 100) != 0) || (nYear % 400) == 0);
}
const TDateTimer TDateTimer::GetCurrentTime(void)
{
#if defined(XMT_WIN_32)
	SYSTEMTIME system_time;
	GetLocalTime(&system_time);
	TIME_ZONE_INFORMATION tmp;
	GetTimeZoneInformation(&tmp);
	TDateTimer t(system_time.wYear, system_time.wMonth, system_time.wDay, system_time.wHour, system_time.wMinute,
		system_time.wSecond, system_time.wMilliseconds, system_time.wDayOfWeek);
	//设置时区信息
	int nIndex = tmp.Bias / (-60);//第8区
	char m_prefix = nIndex > 0 ? 'E' : 'W';
	tstring m_descrption = TFmtstring("% the % th District").arg(m_prefix == 'W' ? "West" : "East").arg(nIndex).c_str();
	t.setTimeZones(nIndex, m_prefix, m_descrption.c_str());
	return t;
#else
	struct timeval ut;	//初始化秒和微妙 当前时区的时间
	//struct timezone us;	//初始化与标准时区的时差 结构体 tz_minuteswest:和格林威治时间差了多少分钟 有问题!!!
	gettimeofday(&ut, NULL);
	TDateTimer t(static_cast<ulonglong>(ut.tv_sec), (double)ut.tv_usec / 1000 / 1000);
	//设置时区信息
	int nIndex = localtime(&ut.tv_sec)->tm_gmtoff / 3600; //时区索引
	char m_prefix = nIndex > 0 ? 'E' : 'W';
	tstring m_descrption = TFmtstring("% the % th District").arg(m_prefix == 'W' ? "West" : "East").arg(nIndex).c_str();
	t.setTimeZones(nIndex, m_prefix, m_descrption.c_str());
	return t;
#endif
}
const TDateTimer& TDateTimer::setTime(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nMilli, int nDayofweek)
{
	if (!TDateTimer::isValidYear(nYear, nMonth, nDay))
		throw XException(TFmtstring("Set DateTime input arg[%-%-%] error").arg(nYear).arg(nMonth).arg(nDay).c_str(), XException::XEP_INVALID_ARGS);
	if (!TDateTimer::isValidTime(nHour, nMinute, nSecond))
		throw XException(TFmtstring("Set DateTime input arg[%:%:%] error").arg(nHour).arg(nMinute).arg(nSecond).c_str(), XException::XEP_INVALID_ARGS);
	m_timStruct.nYear = nYear;
	m_timStruct.nMon = nMonth;
	m_timStruct.nDay = nDay;
	m_timStruct.nHour = nHour;
	m_timStruct.nMin = nMinute;
	m_timStruct.nSec = nSecond;
	m_timStruct.nMiSec = nMilli;
	m_timStruct.nweek = nDayofweek;
	convertToTimet();
	return *this;
}
void TDateTimer::setTimeZones(int nIndex, char mprefix, const char *des)
{
	m_strZones.m_nIndex = nIndex;
	m_strZones.m_prefix = mprefix;
	strcpy(m_strZones.m_description, des);
}

const TDateTimer& TDateTimer::convertToTimet()
{
	if (m_liTime == 0)//需要转化
	{
		struct tm test;
		test.tm_year = m_timStruct.nYear - 1900;
		test.tm_mon = m_timStruct.nMon - 1;
		test.tm_mday = m_timStruct.nDay;
		test.tm_hour = m_timStruct.nHour;
		test.tm_min = m_timStruct.nMin;
		test.tm_sec = m_timStruct.nSec;
		m_liTime = static_cast<ulonglong>(mktime(&test));
	}
	return *this;
}

//Output: Sun Oct 15 2017 00:00:00 GMT+0800 (中国标准时间);
tstring TDateTimer::toString(void) const
{
#if defined(XMT_WIN_32)
	tstring strFormat = TFmtstring("% % % % %:%:% GMT+0%00 (Relative standard time in Greenwich)")
		.arg(m_strWeek[m_timStruct.nweek]).arg(m_strMon[m_timStruct.nMon - 1])
		.arg(TFmtstring::toFixed(m_timStruct.nDay, 2, '0'))
		.arg(TFmtstring::toFixed(m_timStruct.nYear, 4, '0'))
		.arg(TFmtstring::toFixed(m_timStruct.nHour, 2, '0'))
		.arg(TFmtstring::toFixed(m_timStruct.nMin, 2, '0'))
		.arg(TFmtstring::toFixed(m_timStruct.nSec, 2, '0'))
		.arg(m_strZones.m_nIndex)
		.c_str();
	return strFormat;
#else
	struct tm *_t;
	time_t _localt = m_liTime;
	_t = localtime(&_localt);
	tstring strFormat = TFmtstring("% % % % %:%:% GMT+0%00 (Relative standard time in Greenwich)")
		.arg(m_strWeek[_t->tm_wday]).arg(m_strMon[_t->tm_mon])
		.arg(TFmtstring::toFixed(_t->tm_mday, 2, '0'))
		.arg(TFmtstring::toFixed(_t->tm_year + 1900, 4, '0'))
		.arg(TFmtstring::toFixed(_t->tm_hour, 2, '0'))
		.arg(TFmtstring::toFixed(_t->tm_min, 2, '0'))
		.arg(TFmtstring::toFixed(_t->tm_sec, 2, '0'))
		.arg(m_strZones.m_nIndex)
		.c_str();
	return strFormat;
#endif
}
//strFormat %Y - 年份 %M - 月份 %D - 日期
//strFormat %H - 小时 %m - 分钟 %S - 秒数 %s - 毫秒 
//strFormat %w - 星期 
tstring TDateTimer::toFmtString(const char *strFormat) const
{
	tstring _au(strFormat);
	TYPE_TIME _type = TDateTimer::TYPE_24;
#if defined(XMT_WIN_32)
	while (_au.find("%") != tstring::npos)
	{
		if (_au.find("%Y") != tstring::npos)
			_au.replace(_au.find("%Y"), 2, TFmtstring::toFixed(m_timStruct.nYear, 4, '0'));
		if (_au.find("%M") != tstring::npos)
			_au.replace(_au.find("%M"), 2, TFmtstring::toFixed(m_timStruct.nMon , 2, '0'));
		if (_au.find("%D") != tstring::npos)
			_au.replace(_au.find("%D"), 2, TFmtstring::toFixed(m_timStruct.nDay, 2, '0'));
		if (_au.find("%H") != tstring::npos)
			_au.replace(_au.find("%H"), 2, TFmtstring::toFixed(m_timStruct.nHour, 2, '0'));
		else if (_au.find("%h") != tstring::npos)
		{
			int nHour = m_timStruct.nHour > 12 ? m_timStruct.nHour - 12 : m_timStruct.nHour;
			_type = m_timStruct.nHour > 12 ? TDateTimer::TYPE_12_PM : TDateTimer::TYPE_12_AM;
			_au.replace(_au.find("%h"), 2, TFmtstring::toFixed(nHour, 2, '0'));
		}
		if (_au.find("%m") != tstring::npos)
			_au.replace(_au.find("%m"), 2, TFmtstring::toFixed(m_timStruct.nMin, 2, '0'));
		if (_au.find("%S") != tstring::npos)
			_au.replace(_au.find("%S"), 2, TFmtstring::toFixed(m_timStruct.nSec, 2, '0'));
		if (_au.find("%s") != tstring::npos)
			_au.replace(_au.find("%s"), 2, TFmtstring::toFixed(0, 3, '0'));
		if (_au.find("%w") != tstring::npos)
			_au.replace(_au.find("%w"), 2, m_strWeek[m_timStruct.nweek]);
		if (_au.find("%W") != tstring::npos)
			_au.replace(_au.find("%W"), 2, m_strWeekF[m_timStruct.nweek]);
	}
	if (_type == TDateTimer::TYPE_12_AM)
		_au.append(" AM ");
	else if (_type == TDateTimer::TYPE_12_PM)
		_au.append(" PM ");
#else
	struct tm *_t;
	time_t _localt = m_liTime;
	_t = localtime(&_localt);
	while (_au.find("%") != tstring::npos)
	{
		if (_au.find("%Y") != tstring::npos)
			_au.replace(_au.find("%Y"), 2, TFmtstring::toFixed(_t->tm_year + 1900, 4, '0'));
		if (_au.find("%M") != tstring::npos)
			_au.replace(_au.find("%M"), 2, TFmtstring::toFixed(_t->tm_mon + 1, 2, '0'));
		if (_au.find("%D") != tstring::npos)
			_au.replace(_au.find("%D"), 2, TFmtstring::toFixed(_t->tm_mday, 2, '0'));
		if (_au.find("%H") != tstring::npos)
			_au.replace(_au.find("%H"), 2, TFmtstring::toFixed(_t->tm_hour, 2, '0'));
		else if (_au.find("%h") != tstring::npos)
		{
			int nHour = _t->tm_hour > 12 ? _t->tm_hour - 12 : _t->tm_hour;
			_type = _t->tm_hour > 12 ? TDateTimer::TYPE_12_PM : TDateTimer::TYPE_12_AM;
			_au.replace(_au.find("%h"), 2, TFmtstring::toFixed(nHour, 2, '0'));
		}
		if (_au.find("%m") != tstring::npos)
			_au.replace(_au.find("%m"), 2, TFmtstring::toFixed(_t->tm_min, 2, '0'));
		if (_au.find("%S") != tstring::npos)
			_au.replace(_au.find("%S"), 2, TFmtstring::toFixed(_t->tm_sec, 2, '0'));
		if (_au.find("%s") != tstring::npos)
			_au.replace(_au.find("%s"), 2, TFmtstring::toFixed(0, 3, '0'));
		if (_au.find("%w") != tstring::npos)
			_au.replace(_au.find("%w"), 2, m_strWeek[_t->tm_wday]);
		if (_au.find("%W") != tstring::npos)
			_au.replace(_au.find("%W"), 2, m_strWeekF[_t->tm_wday]);
	}
	if (_type == TDateTimer::TYPE_12_AM)
		_au.append(" AM ");
	else if (_type == TDateTimer::TYPE_12_PM)
		_au.append(" PM ");
#endif
	return _au;
}
NAMESPACE_END