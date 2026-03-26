class Solution {
public:
    int longestConsecutive(vector<int>& nums) {
        unordered_set<int> values(nums.begin(), nums.end());
        int best = 0;

        for (int x : values) {
            if (values.count(x - 1)) {
                continue;
            }

            int length = 1;
            int current = x;
            while (values.count(current + 1)) {
                current++;
                length++;
            }

            if (length > best) {
                best = length;
            }
        }

        return best;
    }
};
