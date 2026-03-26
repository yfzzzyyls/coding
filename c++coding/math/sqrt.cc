class Solution {
public:
    int mySqrt(int x) {
        if (x < 2) {
            return x;
        }

        int left = 1;
        int right = x / 2;
        int answer = 0;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            long long square = 1LL * mid * mid;

            if (square == x) {
                return mid;
            }

            if (square < x) {
                answer = mid;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }

        return answer;
    }
};
