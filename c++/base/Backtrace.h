#pragma once
#include <string>
namespace muduo {

std::string GetBackCallStack();

std::string GetSymbolName(const std::string &);

std::string TransLateDemangle(const std::string &);

} // namespace muduo
