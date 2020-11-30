#pragma once

#ifndef _ARGPARSE_H
#define _ARHPARSE_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <string.h>
#include <algorithm>
using namespace std;
namespace argparse
{

    enum DataType
    {
        Type_String,
        Type_Int,
        Type_Double,
        Type_Long,
        Type_Null
    };

    struct option
    {
        option(const char *shortArg, const char *longArg, const char *description, bool bRequired, DataType nType)
            : m_pShortArg(shortArg), m_pLongArg(longArg), m_pDescription(description), m_bRequired(bRequired), m_nType(nType)
        {
        }
        option() : m_pShortArg(nullptr), m_pLongArg(nullptr), m_pDescription(nullptr), m_bRequired(false), m_nType(Type_Null) {}
        const char *m_pShortArg;
        const char *m_pLongArg;
        const char *m_pDescription;
        bool m_bRequired;
        DataType m_nType;
    };
    typedef std::vector<option> optionGroup;

    class SubParse
    {
    public:
        SubParse()
        {
            m_nMaxWidth = 0;
        }
        void add_argument(const char *strShortArg, const char *strLongArg, const char *strDescription, DataType nType = Type_Null, bool bRequired = false)
        {
            int maxSize = 0;
            if (strShortArg != nullptr)
                maxSize += strlen(strShortArg);
            if (strLongArg != nullptr)
                maxSize += strlen(strLongArg);
            m_nMaxWidth = max(maxSize, maxSize + 5);
            innerGroup.push_back(option(strShortArg, strLongArg, strDescription, bRequired, nType));
        }
        friend ostream &operator<<(ostream &os, SubParse &object)
        {
            for (auto &item : object.innerGroup)
            {
                os << setw(2) << setfill(' ') << " ";
                std::string argString;
                if (item.m_pShortArg != nullptr)
                {
                    argString.append("-");
                    argString.append(item.m_pShortArg);
                }
                else
                {
                    argString.append("-");
                }
                argString.append("/");
                if (item.m_pLongArg != nullptr)
                {
                    argString.append("--");
                    argString.append(item.m_pLongArg);
                }
                else
                {
                    argString.append("--");
                }
                os << setw(object.m_nMaxWidth) << argString;
                if (item.m_pDescription != nullptr)
                    os << item.m_pDescription;
                os << std::endl;
            }
            return os;
        }
        void displayArgs(std::ostream &os, bool flags = false)
        {
            if(flags)
                os << " {";
            size_t i = 0;
            for (auto &item : innerGroup)
            {
                
                os << "[";
                if (item.m_pShortArg != nullptr)
                    os << "-" << item.m_pShortArg;
                if (item.m_pLongArg != nullptr)
                {
                    if (item.m_pShortArg)
                        os << " ";
                    os << "--" << item.m_pLongArg;
                    std::string UpLongArg = item.m_pLongArg;
                    std::transform(UpLongArg.begin(), UpLongArg.end(), UpLongArg.begin(), ::toupper);
                    os << " " << UpLongArg;
                }
                os << "]";
                if(i != innerGroup.size()-1)
                    os << " ";
                i++;
                
            }
            if(flags)
                os << "}";
        }

    private:
        int m_nMaxWidth;
        optionGroup innerGroup;
    };
    typedef std::map<std::string, SubParse> optionsGroup;
    class Argparse
    {
    public:
        Argparse(const std::string &strAppName, const std::string &strAppDescription)
            : m_strAppName(strAppName), m_strAppDescription(strAppDescription)
        {
            addSubParse("main");
            add_inner_argument("h", "help", "show this help message and exit", Type_Null, false);
            add_inner_argument("v", "version", "show this app version", Type_Null, false);
        }
        void PrintHelp(std::ostream &os = std::cout)
        {
            std::string appName = "./" + m_strAppName;
            os << m_strAppDescription << "\r\n"
               << "usage: "
               << std::left << setw(appName.size() + 2) << appName;

            for (auto item : m_OptionsGroup)
            {
                if(item.first == "main")
                    item.second.displayArgs(os);
                else
                    item.second.displayArgs(os, true);
            }
            os << "\r\n\r\n"
               << "optional arguments:\r\n";
            // main
            for (auto &item : m_OptionsGroup)
            {
                if (item.first != "main")
                    os << "\r\nsubparse argument " << item.first << "\r\n";
                os << item.second;
            }
        }
        void add_argument(const char *strShortArg, const char *strFullArg, const char *strDescription, DataType nType = Type_String, bool bRequired = false)
        {
            add_inner_argument(strShortArg, strFullArg, strDescription, nType, bRequired);
        }
        void ParseArgs(int argc, char **argv)
        {
        }
        SubParse &addSubParse(const std::string &parseName)
        {
            m_OptionsGroup.insert(std::pair<std::string, SubParse>(parseName, SubParse()));
            return m_OptionsGroup.at(parseName);
        }

    protected:
        void add_inner_argument(const char *strShortArg, const char *strLongArg, const char *strDescription, DataType nType, bool bRequired)
        {
            m_OptionsGroup.at("main").add_argument(strShortArg, strLongArg, strDescription, nType, bRequired);
        }

    protected:
        std::string m_strAppName;
        std::string m_strAppDescription;
        optionsGroup m_OptionsGroup;
    };

} // namespace argparse

#endif
