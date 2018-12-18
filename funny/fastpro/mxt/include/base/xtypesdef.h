#ifndef _X_DATEDEF_H_2018_12_18
#define _X_DATEDEF_H_2018_12_18

#include "xmtdef.h"
#include "base/xstring.h"
NAMESPACE_BEGIN

//Data Time Def
struct timeStruct
{
public:
	int nYear;		//年 
	int nMon;		//月 1-12
	int nDay;		//日 1-31
	int nHour;		//时 0-23
	int nMin;		//分 0-59
	int nSec;		//秒 0-59
	int nMiSec;		//毫秒 0-999
	int nMicroSecond; // 0-999
	int nweek;		//星期 [0-6] 日一二三四五六
	int nDayOfYear;
	tstring toString() const
	{
		return TFmtstring("%/%/% %:%:%.% %").arg(nYear).arg(nMon).arg(nDay).arg(nHour).arg(nMin).arg(nSec).arg(nMiSec).arg(nweek).c_str();
	}
};
struct timeZones
{
public:
	size_t m_nIndex;		// 时区序号
	char m_prefix;			// E/W 东/西
	char m_description[20]; // 描述信息
};

//XVariant




NAMESPACE_END

#endif
