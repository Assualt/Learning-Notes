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
		TYPE_NONE = 0,			//null
		TYPE_BOOL = 1,			//bool
		TYPE_BOOLFALSE = 2,		//
		TYPE_INT = 3,			//int
		TYPE_LONG = 4,			//long
		TYPE_DOUBLE = 5,		//double
		TYPE_STR = 6,			//tstring
		TYPE_WSTR = 7,			//twstring
		TYPE_CSTR = 8,			//cstring
		TYPE_BIN = 9,			//binary stream
		TYPE_DATETIME = 10,		//datetime
		TYPE_ARRAY = 11,		//arry
		TYPE_STRUCT = 12,		//struct
		TYPE_OBJECT = 13,		//object

		TYPE_WSTRBE = 14,		
		TYPE_WSTRLE = 15,

		TYPE_DEF_END
	};

	typedef std::vector<TVariant> typeArry;			//list
	typedef std::map<tstring, TVariant> typeStruct; //struct
	
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
