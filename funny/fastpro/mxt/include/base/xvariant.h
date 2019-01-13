#ifndef _X_VARIANT_H_2018_12_18
#define _X_VARIANT_H_2018_12_18

#include "xmtdef.h"
#include <vector>
#include <map>
#include "base/xstring.h"

NAMESPACE_BEGIN


class TVariant
{
public:
	enum VALUE_TYPE
	{
		TYPE_NONE = 0,
		TYPE_BOOL = 1,
		TYPE_BOOLFALSE = 2,
		TYPE_INT = 3,
		TYPE_LONG = 4,
		TYPE_DOUBLE = 5,
		TYPE_STR = 6,
		TYPE_WSTR = 7,
		TYPE_CSTR = 8,
		TYPE_BIN = 9,
		TYPE_DATETIME = 10,
		TYPE_ARRAY = 11,
		TYPE_STRUCT = 12,
		TYPE_OBJECT = 13,

		TYPE_WSTRBE = 14,
		TYPE_WSTRLE = 15,

		TYPE_DEF_END
	};

	typedef std::vector<TVariant> typeArry;			//Ë³Ðò±í
	typedef std::map<tstring, TVariant> typeStruct; //Ê÷½á¹¹
	
public:
	//static
	static const TVariant g_varNULL;
	static const char * getTypeString(VALUE_TYPE nType);
	static VALUE_TYPE getTypeValue(const char * pTypeString);

public:
	//!explicit
	TVariant(void);
	TVariant(const TVariant &tVar);
	TVariant(VALUE_TYPE nValueType, bool boLockType = true);
	



};


NAMESPACE_END
#endif
