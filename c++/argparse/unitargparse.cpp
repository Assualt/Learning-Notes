#include "argparse.hpp"

using namespace std;

int main(int argc, char **argv)
{

    argparse::Argparse parse("program", "This is a programs description");
    parse.add_argument("u", "university", "this is a university name");
    auto &subParse = parse.addSubParse("rise");
    subParse.add_argument("d", "dest", "This is destion office OK", argparse::Type_Null, false);
    subParse.add_argument("o", "operation", "This is destiond office OK", argparse::Type_Null, false);
    parse.ParseArgs(argc, argv);

    parse.PrintHelp();

    return 0;
}