#include "xmtdef.h"
#include "base/xstring.h"
#include "base/xtoken.h"
#include "base/xstringbuilder.h"
#include "conf/xconfmgrbase.h"
using namespace NAMESPACE_NAME;
int main(int agrc, char *argv[])
{
    std::cout << "Test For TFmtString........." << std::endl;
    std::cout << "............................." << std::endl;

	std::cout << TFmtstring().c_str() << std::endl;
	std::cout << TFmtstring("arg1:% arg2:% arg3:% arg4:% arg5:%")
					 .arg("helloworld")
					 .arg(1)
					 .arg('c')
					 .arg(123.456)
					 .arg(123123123)
					 .c_str()
			  << std::endl;
	std::cout << TFmtstring::toFixed(151, 5, 'A') << std::endl;

    std::cout << "Test For tstringbuilder ....." << std::endl;
    std::cout << "............................." << std::endl;
	tstringbuilder builder;
	builder.append("hello world").append(123);

	std::cout << builder.toString() << ":" << builder.size() << std::endl;
	builder.setCharAt(4,'B');
	std::cout << builder.toString() << ":" << builder.size() << std::endl;
	builder.setCharAt(6,'B');
	std::cout << builder.toString() << ":" << builder.size() << std::endl;
	builder.setCharAt(8,'B');
	std::cout << builder.toString() << ":" << builder.size() << std::endl;

	return 0;
}
