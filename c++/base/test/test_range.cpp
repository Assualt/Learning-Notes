#include "base/Range.h"
#include <vector>
using namespace muduo::base;

template <class T> struct IsSomeString : std::false_type {};

template <typename Alloc> struct IsSomeString<std::basic_string<char, std::char_traits<char>, Alloc>> : std::true_type {};

int main(int argc, char const *argv[]) {

    std::string text = "Hello World";

    StringPiece c(text.data(), 4);

    std::cout << c << std::endl;

    std::cout << c.start() << std::endl;

    std::cout << c.startsWith('h') << std::endl;
    std::cout << c.startsWith("he") << std::endl;

    std::cout << c.startsWith('H') << std::endl;
    std::cout << c.startsWith("He") << std::endl;

    std::vector<int> test{1, 2, 3, 4};

    Range<std::vector<int>::iterator> test_1(test.begin(), test.end());

    std::cout << test_1.front() << std::endl;
    std::cout << test_1.back() << std::endl;

    return 0;
}
