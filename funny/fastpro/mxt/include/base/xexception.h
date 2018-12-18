#ifndef _X_EXCEPTION_2018_12_14
#define _X_EXCEPTION_2018_12_14

#include "xmtdef.h"

#include "string.h"
#include "xstring.h"

#include <exception>
#include <iostream>

using namespace std;

NAMESPACE_BEGIN

const char m_strErrorMsg[6][20] = { "NORMAL","Normal Error","Invalid Args","Handler Error","Internal Error","Unknown Error" }; 

class XException:public std::exception
{
public:
	enum XEP_CODE
	{
		XEP_NORMAIL,			//正常
		XEP_ERROR,				//一般错误
		XEP_INVALID_ARGS,		//错误参数
		XEP_HADNLER_ERROR,		//处理错误
		XEP_INTERNAL_ERROR,		//内部错误
		XEP_UNKNOWN_ERROR		//未知错误
	};
	XException(const char * what,XEP_CODE code = XEP_ERROR) throw()
	{
		eCode = code;
		if (NULL == what)
		{
			eMssage = new char[1];
			eMssage[0] = '\0';
		}
		else
		{
			size_t _len = strlen(what);
			eMssage = new char[_len + 1];
			strncpy(eMssage, what, _len);
			eMssage[_len] = '\0';
		}
	}
	const char * what() const throw()
	{
		return eMssage;
	}
	XEP_CODE getErrorCode(void) const throw()
	{
		return eCode;
	}
	const char * getErrorCodeToString() const throw()
	{
		return m_strErrorMsg[eCode];
	}

	~XException() throw()
	{
		delete[] eMssage;
	}
	friend ostream & operator << (ostream & out, XException & e)
	{
		out << e.what();
		return out;
	}
private:
	char * eMssage;
	XEP_CODE eCode;
	static char m_strErrorMsg[6][20];
};
NAMESPACE_END


#endif // !_X_EXCEPTION_2018_12_14
