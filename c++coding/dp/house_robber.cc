#include <algorithm>
#include <vector>

class Solution {
public:
    int rob(std::vector<int>& nums) {
        int robPrev = 0;
        int skipPrev = 0;

        for (int money : nums) {
            int newRob = skipPrev + money;
            int newSkip = std::max(skipPrev, robPrev);

            robPrev = newRob;
            skipPrev = newSkip;
        }

        return std::max(robPrev, skipPrev);
    }
};
