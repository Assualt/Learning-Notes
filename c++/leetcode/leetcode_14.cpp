#include <iostream>
#include <vector>
using namespace std;
#define INT_MAX (2 ^ 31 - 1)
class Solution {
public:
    string longestCommonPrefix(const vector<string> &strs) {
        string s;
        int    minLength = INT_MAX;
        for (size_t i = 0; i < strs.size(); ++i) {
            if (minLength >= strs[ i ].size())
                minLength = strs[ i ].size();
        }
        for (size_t i = 0; i < minLength; ++i) {
            char ch = strs[ 0 ][ i ];
            for (size_t j = 1; j < strs.size(); ++j) {
                if (strs[ j ][ i ] != ch)
                    return s;
            }
            s.push_back(ch);
        }
        return s;
    }
};

int main(int argc, char const *argv[]) {

    Solution s;

    std::cout << "Prefix" << s.longestCommonPrefix({"flower","flow","flight"}) << std::endl;

    return 0;
}
