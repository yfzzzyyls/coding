#include <queue>
#include <vector>

class Solution {
public:
    int findKthLargest(std::vector<int>& nums, int k) {
        std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;

        for (int num : nums) {
            minHeap.push(num);

            if (static_cast<int>(minHeap.size()) > k) {
                minHeap.pop();
            }
        }

        return minHeap.top();
    }
};
