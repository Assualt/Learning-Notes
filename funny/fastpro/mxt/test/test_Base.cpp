#include "xmtdef.h"
#include "base/xstring.h"
#include "base/xstringbuilder.h"
#include "base/xtoken.h"
#include "base/xdatetimer.h"
using namespace xmt;
int main(int agrc, char *argv[])
{
	/*
	tstring s("hello");
	std::cout << s.size() << std::endl;
	tstring str = TFmtstring("strformat:% authuser:% from:% hdrfrom:%").arg("authus").arg(2.3).arg("asdas").arg("asd12").c_str();
	std::cout << str << std::endl;

	tstring src0 = "abc,dea,adssad,ed,ad";
	std::list<tstring> ret;
	TStringHelper::split(ret, src0, ',');
	std::list<tstring>::iterator iter(ret.begin()), iterend(ret.end());
	for (; iter != iterend; iter++)
	{
		std::cout << *iter << std::endl;
	}
	tstring sp = TFmtstring("strFormat:% arg:% c:% d:% e:% f:% g:%").arg(1).arg(1.0).arg(1.00).arg("1").arg('c').arg(10000000).arg("c").c_str();
	std::cout << "sp:" << sp << std::endl;

	tstring src = " adbcdef  asdasd ";
	std::cout << "src:" << src << "size:" << src.size() << std::endl;
	TStringHelper::ltrim(src);
	std::cout << "src:" << src << "size:" << src.size() << std::endl;
	tstring src1 = " adbcdef  asdasd ";
	TStringHelper::rtrim(src1);
	std::cout << "src1:" << src1 << "size:" << src1.size() << std::endl;
	tstring src2 = " adbcdef  asdasd ";
	TStringHelper::trim(src2);
	std::cout << "src2:" << src2 << "size:" << src2.size() << std::endl;

	tstring strReplace = "abcdadas,asd";

	TStringHelper::replaceAll(strReplace, 'a', 'z');
	std::cout << "strReplace:" << strReplace << std::endl;

	tstring match = "abcdefghiabc";

	std::cout << TStringHelper::startWith(match, "abc") << std::endl;
	std::cout << TStringHelper::startWith(match, "abd") << std::endl;
	std::cout << TStringHelper::endWith(match, "abc") << std::endl;
	std::cout << TStringHelper::endWith(match, "abac") << std::endl;

	std::cout << "------------test toString" << std::endl;
	//std::cout << sb.deleter(1, 3).toString() << std::endl;
	//std::cout << sb.insert(5, 'c').toString() << std::endl;
	//std::cout << sb.replace(1, 3, "hover").toString() << std::endl;
	//std::cout << sb.substr(1, 4) << std::endl;

	std::cout << toString("hello world") << ":" << toString("hello world").length() << std::endl;
	std::cout << toString('c') << ":" << toString('c').length() << std::endl;
	std::cout << toString(123) << ":" << toString(123).length() << std::endl;
	std::cout << toString(123.00) << ":" << toString(123.01).length() << std::endl;
	std::cout << toString(-1) << ":" << toString(-1).length() << std::endl;

	tstringbuilder sb("hello world");
	sb.append("123").append("abcdef").append('c').append(123);
	std::cout << sb.toString() << ":" << sb.size() << std::endl;
	sb.deleter(1, 3);
	std::cout << sb.toString() << ":" << sb.size() << std::endl;
	std::cout << "deleter(1,12)-----------------------------" << std::endl;
	sb.deleter(1, 12);
	std::cout << sb.toString() << ":" << sb.size() << std::endl;
	sb.deleter(0);
	std::cout << sb.toString() << ":" << sb.size() << std::endl;

	sb.insert(0, 'c');
	std::cout << sb.toString() << ":" << sb.size() << std::endl;
	sb.insert(0, "hello world");
	std::cout << sb.toString() << ":" << sb.size() << std::endl;

	sb.replace(1, 3, "cc");
	std::cout << sb.toString() << ":" << sb.size() << std::endl;
	sb.replaceAll('c', 'd');
	std::cout << sb.toString() << ":" << sb.size() << std::endl;

	sb.getCharSize('d');

	tstring h;
	TStringHelper::toBytes('c', h);
	std::cout << h << std::endl;
	tstring h1;
	TStringHelper::toBytes("abc", h1);
	std::cout << h1 << std::endl;
	h1 += "asd";
	std::list<tstring> h2;
	TStringHelper::splitBytes(h2, h1, 8);

	std::cout << "[a:bd;da,asd:asd,asd]" << std::endl;
	TTokens t("[a:bd;da,asd:asd,asd]","[,:;]");
	t.split();
	tstring strOut;
	while (t.hasNext(strOut))
	{
		std::cout << "strOut:" << strOut << std::endl;
	}

	struct TDateTimer::timeStruct tf;
	time_t ss = 86400;
	TDateTimer::getDiffTime2Struct(tf, ss);

	std::string str = "";

	std::cout << "123" << std::endl;

	int a = 123, b = 231, c = 1231, d = 987;

	tstring strFormat = TFmtstring("% % % % %").arg(a).arg(b).arg(c).arg(d).c_str();
	std::cout << "c:" << strFormat << std::endl;

	std::cout << TFmtstring::format("1", 2, '0') << std::endl;
	std::cout << TFmtstring::format("1", 2, '0', false) << std::endl;

	std::cout << TFmtstring::format("12sadasd", 5, '0') << std::endl;
	std::cout << TFmtstring::format("12sadasd", 5, '0', false) << std::endl;
	*/
	tstring strFormat1 = " %w:%Y-%M-%D %h:%m:%S.%s - week %W: %W";
	TDateTimer tap = TDateTimer::GetCurrentTime();
	std::cout << tap.toString() << std::endl;
	std::cout << tap.toFmtString(strFormat1.c_str()) << std::endl;
 


 

	return 0;
}