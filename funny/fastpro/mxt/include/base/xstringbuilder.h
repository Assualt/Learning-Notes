#ifndef _X_STRINGBUILDER_H
#define _X_STRINGBUILDER_H

#include "xmtdef.h"
#include "xstring.h"
#include <list>
#include <cstring>
#include <numeric>
#include <algorithm>


NAMESPACE_BEGIN

class tstringbuilder
{
	typedef std::list<tstring> container_t;
	typedef typename tstring::size_type size_type;
	typedef typename std::list<tstring>::const_iterator iter_t_con;
	typedef typename std::list<tstring>::iterator iter_t;

private:
	container_t m_Data;
	size_type m_totalSize;

	void Append(const tstring &src)
	{
		m_Data.push_back(src);
		m_totalSize += src.size();
	}
	void Append(const char * src)
	{
		m_Data.push_back(tstring(src));
		m_totalSize += strlen(src);
	}
	void Append(bool src)
	{
		tstring a = src == true ? "true" : "false";
		m_Data.push_back(a);
		m_totalSize += a.size();
	}
	void Append(float src)
	{
		m_Data.push_back(to_string(src));
		m_totalSize += to_string(src).size();
	}
	void Append(int src)
	{
		m_Data.push_back(to_string(src));
		m_totalSize += to_string(src).size();
	}
	void Append(double src)
	{
		m_Data.push_back(to_string(src));
		m_totalSize += to_string(src).size();
	}
	void Append(long src)
	{
		m_Data.push_back(to_string(src));
		m_totalSize += to_string(src).size();
	}
	void Append(long long src)
	{
		m_Data.push_back(to_string(src));
		m_totalSize += to_string(src).size();
	}
public:
	tstringbuilder()
	{
		m_totalSize = 0;
	}
	explicit tstringbuilder(const tstring &src)
	{
		if (!src.empty())
			m_Data.push_back(src);
		m_totalSize = src.size();
	}
	explicit tstringbuilder(const char *src)
	{
		if (NULL != src)
			m_Data.push_back(src);
		m_totalSize = strlen(src);
	}
	~tstringbuilder()
	{
		m_totalSize = 0;
		m_Data.clear();
	}
	size_type size()
	{
		return m_totalSize;
	}
	size_type length()
	{
		return m_totalSize;
	}
	template <typename val_type>
	tstringbuilder& append(const val_type &val)
	{
		Append(val);
		return *this;
	}
	char charAt(size_t index)
	{
		return toString()[index];
	}

	tstring toString() const
	{
		tstring result;
		result.reserve(m_totalSize + 1);
		iter_t_con iter(m_Data.begin()), iterend(m_Data.end());
		for (; iter != iterend; iter++)
			result += *iter;
		return result;
	}

	tstring substr(size_t _pBegin = 0, size_t _len = tstring::npos)
	{
		return toString().substr(_pBegin, _len);
	}
	tstringbuilder & deleter(size_t _pBegin, size_t _len)
	{
		if (_pBegin == 0)//clear all
		{
			m_Data.clear();
			m_totalSize = 0;
			return *this;
		}
		if (_pBegin > m_totalSize - 1 || (_pBegin + _len) > m_totalSize)
			throw XException(TFmtstring("Can't delete the % length,while the total length is %").arg(_len).arg(m_totalSize).c_str());
		iter_t iter(m_Data.begin()), iterend(m_Data.end());
		size_t _posBegin = 0, _posEnd = 0, _pEnd = _len + _pBegin;
		bool bFirst = true;
		m_totalSize -= _len;
		for (; iter != iterend; iter++)
		{
			size_t _isize = iter->size();
			_posEnd = _posBegin + iter->size();
			if (_pBegin >= _posBegin && _pEnd <= _posEnd)// deleter length belong to one iter
			{
				if (_pEnd == _posEnd)
				{
					iter->erase(_pBegin - _posBegin);
					m_totalSize -= (_posEnd - _pBegin);
				}
				else
				{
					iter->erase(_pBegin - _posBegin, _pEnd - _pBegin);
					return *this;
				}
			}
			else if (_pBegin >= _posBegin && _pEnd > _posEnd)
			{
				iter->erase(_pBegin - _posBegin, _posEnd - _pBegin + 1);
			}
			else {}
			_pBegin = bFirst ? _pBegin + _isize - 1 : _pBegin + _isize;
			_posBegin = _posEnd;
			bFirst = false;
		}
		return *this;
	}
	tstringbuilder & deleter(size_t _pBegin)
	{
		return deleter(_pBegin, m_totalSize - _pBegin);
	}
	tstringbuilder & insert(size_t _pos, const char* c)
	{
		if (_pos > m_totalSize - 1)
			throw XException(TFmtstring("can't insert such char:% at pos:%").arg(c).arg(_pos).c_str());
		if (m_totalSize == 0)
			return append(c);
		iter_t iter(m_Data.begin()), iterend(m_Data.end());
		m_totalSize += strlen(c);
		size_t _posBegin = 0, _posEnd = 0;
		for (; iter != iterend; iter++)
		{
			_posEnd = _posBegin + iter->size() - 1;
			if (_pos >= _posBegin && _pos <= _posEnd)
			{
				iter->insert(_pos - _posBegin, c);
				break;
			}
		}
		return *this;
	}
	tstringbuilder & insert(size_t _pos, const char c)
	{
		return insert(_pos, tstring(1, c).c_str());
	}
	tstringbuilder & replace(size_t _pos, size_t _len, const char c)
	{
		return replace(_pos, _len, tstring(1, c).c_str());
	}
	tstringbuilder & replace(size_t _pos, size_t _len, const char *s)
	{
		deleter(_pos, _len);
		insert(_pos, s);
		return *this;
	}
	tstringbuilder & replaceAll(const char src, const char dest)
	{
		if (m_totalSize == 0)
			return *this;
		iter_t iter(m_Data.begin()), iterend(m_Data.end());
		for (; iter != iterend; iter++)
			std::transform(iter->begin(), iter->end(), iter->begin(), [=](unsigned char ch)->char {return ch == src ? dest : ch; });
		return *this;
	}
	size_t getCharSize(const char c)const
	{
		tstring tmp = toString();
		size_t ret = std::count_if(tmp.begin(), tmp.end(), [=](char ch) {return ch == c ? true : false; });
		return ret;
	}
};
NAMESPACE_END

#endif

