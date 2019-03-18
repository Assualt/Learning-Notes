#include "xmtdef.h"
#include "base/xstring.h"
#include "base/xtoken.h"

using namespace NAMESPACE_NAME;
int main(int agrc, char *argv[])
{
	std::cout << "hello world" << std::endl;
	std::cout << "TFmtstring" << std::endl;

	std::cout << TFmtstring().c_str() << std::endl;
	std::cout << TFmtstring("arg1:% arg2:% arg3:% arg4:% arg5:%")
	.arg("helloworld").arg(1).arg('c').arg(123.456).arg(123123123).c_str() << std::endl;
	std::cout << TFmtstring::toFixed(151,5,'A') << std::endl;
	

	return 0;
}
