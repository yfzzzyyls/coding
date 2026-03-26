#include <vector>

class Solution {
public:
    int maxSubArray(std::vector<int>& nums) {
        int currentSum = nums[0];
        int bestSum = nums[0];

        for (int i = 1; i < static_cast<int>(nums.size()); i++) {
            currentSum = std::max(nums[i], currentSum + nums[i]);
            bestSum = std::max(bestSum, currentSum);
        }

        return bestSum;
    }
};
