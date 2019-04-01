#include "conf/xconfmgrbase.h"

NAMESPACE_BEGIN

bool TInIConfigureManager::onInit()
{
	return initIniFiles();
}

bool TInIConfigureManager::initIniFiles()
{
	std::vector<tstring>::iterator it(m_strFileNames.begin()), itend(m_strFileNames.end());
	for (; it != itend; it++)
	{
		std::ifstream fin(it->c_str(), ios::in);
		if (!fin.is_open())
			throw TConfigureException(TFmtstring("Can't open such % file").arg(it->c_str()).c_str());
		TFileListMap t;
		initSingerFile(t, fin);
		std::cout << "handing " << *it << "file" << t.size() << std::endl;
		m_mKVMap.insert(std::pair<tstring, TFileListMap>(*it, t));
		fin.close();
	}
	std::cout << "Init ini Files End" << std::endl;

	TFilesListMap::iterator fileIter(m_mKVMap.begin()), fileIterEnd(m_mKVMap.end());
	for (; fileIter != fileIterEnd; fileIter++)
	{
		std::cout << "Files:" << fileIter->first << std::endl;
		TFileListMap::iterator sectionMapIter(fileIter->second.begin()), sectionMapIetrEnd(fileIter->second.end());
		for (; sectionMapIter != sectionMapIetrEnd; sectionMapIter++)
		{
			std::cout << "[" << sectionMapIter->first << "]" << std::endl;
			std::map<tstring, tstring>::iterator kvIter(sectionMapIter->second.begin()), kvIterEnd(sectionMapIter->second.end());
			for (; kvIter != kvIterEnd; kvIter++)
			{
				std::cout << kvIter->first << "=" << kvIter->second << std::endl;
			}
		}
		std::cout << "-----------" << std::endl;
	}
	return true;
}

void TInIConfigureManager::initSingerFile(TFileListMap &t, std::ifstream &in)
{
	bool bNext = true;
	tstring line;
	while (1)
	{
		if (in.eof())
			break;
		if (bNext)
			getline(in, line);
		TStringHelper::trim(line);
		if (TStringHelper::startWith(line, "#") || line.empty())
			continue;
		if (TStringHelper::startWith(line, "[") && line.find("]") != tstring::npos && line.find("]") != 1)
		{
			std::map<tstring, tstring> kvMap;
			tstring strKey = line.substr(1, line.find("]") - 1); // Section Name
			std::cout << "seky:" << strKey << std::endl;
			tstring sectionLine;
			bNext = true;
			while (getline(in, sectionLine))
			{
				TStringHelper::trim(sectionLine);
				if (sectionLine.empty() || TStringHelper::startWith(sectionLine, "#"))
					continue;
				if (TStringHelper::startWith(sectionLine, "[") && line.find("]") != tstring::npos && line.find("]") != 1) // Found Next Section
				{
					bNext = false;
					line = sectionLine;
					std::cout << "strKey" << kvMap.size() << " " << strKey << std::endl;
					t[strKey] = kvMap;
					break;
				}
				else if(!in.eof())
				{
					size_t nBegin = sectionLine.find("=");
					size_t nPosBegin = 0;
					size_t nPosEnd = 0;
					int nCnt = std::count_if(sectionLine.begin(), sectionLine.end(), [](int ch)->bool {return ch == '"'; });
					if (nCnt >= 2)
					{
						nPosBegin = sectionLine.find("\"", nBegin + 1);
						nPosEnd = sectionLine.find("\"", nPosBegin + 1);
						tstring strSectionKey = sectionLine.substr(0, nBegin);
						TStringHelper::trim(strSectionKey);
						tstring strSectionVal = sectionLine.substr(nPosBegin + 1, nPosEnd - nPosBegin - 1);
						TStringHelper::trim(strSectionVal);
						kvMap[strSectionKey] = strSectionVal;
						std::cout << "key:" << strSectionKey << " val:" << strSectionVal << " size:" << kvMap.size() << std::endl;
					}
					if (nPosEnd == 0)
						continue;
				}
				if (in.eof())
				{
					t[strKey] = kvMap;
					return;
				}
			}
		}
	}
}


NAMESPACE_END

