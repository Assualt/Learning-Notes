#include "xmtdef.h"
#include "xstring.h"
NAMESPACE_BEGIN

class TTokens
{
	typedef std::list<tstring> tstringList;
public:
	TTokens() :m_strSrc(TStringHelper::m_strEmptyString), m_strDel(TStringHelper::m_strEmptyString) {}
	TTokens(const char *lpszSrc, const char *lpszDel, size_t _pBegin = 0, size_t _pEnd = std::basic_string<char>::npos) :m_strDel(lpszDel)
	{
		if (_pBegin > strlen(lpszSrc) - 1)
			throw XException(TFmtstring("% arg(_pBegin) error").arg(__FUNCTION__).c_str(),XException::XEP_INVALID_ARGS);
		m_strSrc = tstring(lpszSrc).substr(_pBegin, _pEnd - _pBegin);
	}
	void setDelimiter(const char *lpszDel)
	{
		if (NULL == lpszDel)
			return;
		m_strDel = lpszDel;
	}
	void split(void);
	template <typename typeChar>
	bool hasNext(typeChar & strOutput)
	{
		strOutput = *m_iter;
		if (m_iter == m_list.end())
			return false;
		m_iter++;
		return true;
	}
private:
	tstring m_strSrc;
	tstring m_strDel;
	tstringList m_list;
	tstringList::iterator m_iter;
};

NAMESPACE_END
