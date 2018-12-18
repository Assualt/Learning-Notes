#include "base/xtoken.h"

NAMESPACE_BEGIN

void TTokens::split(void)
{
	m_list.clear();
	if (m_strSrc.empty() || m_strDel.empty())
		return;
	size_t _pos = 0, _len = m_strSrc.size(), _pBegin = 0;
	for (; _pos < _len; _pos++)
	{
		size_t _tpos = 0, _tlen = m_strDel.size();
		for (; _tpos < _tlen; _tpos++)
		{
			if (m_strSrc[_pos] == m_strDel[_tpos])
			{
				m_list.push_back(m_strSrc.substr(_pBegin, _pos - _pBegin));
				_pBegin = _pos + 1;
				break;
			}
		}
	}
	m_iter = m_list.begin();
}

NAMESPACE_END

