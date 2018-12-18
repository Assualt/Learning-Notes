#ifndef _X_DATEDEF_H_2018_12_18
#define _X_DATEDEF_H_2018_12_18

#include "xmtdef.h"
#include "base/xstring.h"
NAMESPACE_BEGIN

//Data Time Def
struct timeStruct
{
public:
	int nYear;		//�� 
	int nMon;		//�� 1-12
	int nDay;		//�� 1-31
	int nHour;		//ʱ 0-23
	int nMin;		//�� 0-59
	int nSec;		//�� 0-59
	int nMiSec;		//���� 0-999
	int nMicroSecond; // 0-999
	int nweek;		//���� [0-6] ��һ����������
	int nDayOfYear;
	tstring toString() const
	{
		return TFmtstring("%/%/% %:%:%.% %").arg(nYear).arg(nMon).arg(nDay).arg(nHour).arg(nMin).arg(nSec).arg(nMiSec).arg(nweek).c_str();
	}
};
struct timeZones
{
public:
	size_t m_nIndex;		// ʱ�����
	char m_prefix;			// E/W ��/��
	char m_description[20]; // ������Ϣ
};

//XVariant




NAMESPACE_END

#endif
