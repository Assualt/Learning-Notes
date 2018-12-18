#ifndef _X_DATETIMER_H_2018_12_17
#define _X_DATETIMER_H_2018_12_17

#include "xmtdef.h"
#include "xstring.h"
#include "base/xtypesdef.h"

#ifdef XMT_WIN_32
#include "time.h"
#else
#include <sys/time.h>
#endif
NAMESPACE_BEGIN

class TDateTimer
{
public:
	enum TYPE_TIME
	{
		TYPE_12_AM,
		TYPE_12_PM,
		TYPE_24
	};
	enum TYPE_COMPARE
	{
		TYPE_BEFORE,
		TYPE_EQUALS,
		TYPE_AFTER
	};
	//return current time
	static const TDateTimer GetCurrentTime(void);
	//return timeStruct
	static struct timeStruct & getDiffTime2Struct(struct timeStruct &t, time_t _diff)
	{
		t.nMiSec = 0;
		t.nSec = _diff % 60;
		t.nMin = _diff % 3600 / 60;
		t.nHour = _diff % 86400 / 3600;
		t.nDay = _diff / 86400;
		t.nYear = 0;
		return t;
	}

public:
	static char m_strMon[12][10];
	static char m_strWeek[7][10];
	static char m_strMonF[12][20];
	static char m_strWeekF[7][20];
public:
	//TDateTime Default constructor
	TDateTimer(void) :m_liTime(0), m_mSecond(0.00) {};
	//!Copy constructor
	TDateTimer(const TDateTimer& _t) :m_liTime(_t.m_liTime), m_mSecond(_t.m_mSecond) {}
	//!Use ulonglong to init
	explicit TDateTimer(const ulonglong dataSrc, const double datauSecond) :m_liTime(dataSrc), m_mSecond(datauSecond) {}
	//!Use time_t to init
	explicit TDateTimer(const time_t _tSrc) :m_liTime(0), m_mSecond(0.00) {}
	TDateTimer(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nMilli, int nDayofWeek)
		: m_liTime(0), m_mSecond(static_cast<double>(nMilli))
	{
		setTime(nYear, nMonth, nDay, nHour, nMinute, nSecond, nMilli, nDayofWeek);
	}
public:
	bool isNull(void) const { return m_liTime == 0; }
	void setNull(void) { m_liTime = 0; }
	TDateTimer& operator=(const TDateTimer& dateSrc) { m_liTime = dateSrc.m_liTime; return *this; }
	TYPE_COMPARE compare(const TDateTimer & date) const
	{
		if (m_liTime < date.m_liTime)
			return TDateTimer::TYPE_BEFORE;
		else if (m_liTime > date.m_liTime)
			return TDateTimer::TYPE_AFTER;
		return TDateTimer::TYPE_EQUALS;
	}
	bool operator==(const TDateTimer& date) const { return m_liTime == date.m_liTime; }
	bool operator!=(const TDateTimer& date) const { return m_liTime != date.m_liTime; }
	bool operator<=(const TDateTimer& date) const { return m_liTime <= date.m_liTime; }
	bool operator>=(const TDateTimer& date) const { return m_liTime >= date.m_liTime; }
	bool operator< (const TDateTimer& date) const { return m_liTime < date.m_liTime; }
	bool operator> (const TDateTimer& date) const { return m_liTime > date.m_liTime; }

	//格式化输出
	tstring toString(void) const;
	tstring toFmtString(const char *strFormat)const;
	struct timeZones & getTimeZones() { return m_strZones; }
	struct timeStruct & getTimeStruct() { return m_timStruct; }
private:
	const TDateTimer& convertToTimet(); //转化为 time_t 存放于 m_liTime 
	const TDateTimer& setTime(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond, int nMilli, int nDayofWeek);
	void setTimeZones(int nIndex, char mprefix, const char *des);
	//check the date
	static bool isValidYear(int nYear, int nMon, int nDay);
	static bool isValidTime(int nHour, int nMin, int nSec, TYPE_TIME AmPm = TYPE_24);
	static bool isLeapYear(int nYear);
private:
	ulonglong m_liTime; //秒 存放一个格林威治 标准时间值  time_t
	double m_mSecond;   //毫秒级 
	struct timeStruct  m_timStruct;
	struct timeZones m_strZones;
};
NAMESPACE_END

#endif
