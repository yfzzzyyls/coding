class Solution {
public:
    vector<vector<int>> merge(vector<vector<int>>& intervals) {
        if (intervals.empty()) {
            return {};
        }

        sort(intervals.begin(), intervals.end());

        vector<vector<int>> result;
        result.push_back(intervals[0]);

        for (int i = 1; i < intervals.size(); i++) {
            int start = intervals[i][0];
            int end = intervals[i][1];

            if (start <= result.back()[1]) {
                result.back()[1] = max(result.back()[1], end);
            } else {
                result.push_back(intervals[i]);
            }
        }

        return result;
    }
};
