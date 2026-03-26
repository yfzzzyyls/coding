// The n-queens puzzle is the problem of placing n queens on an n x n chessboard such that no two queens attack each other.

// Given an integer n, return the number of distinct solutions to the n-queens puzzle.

 

// Example 1:


// Input: n = 4
// Output: 2
// Explanation: There are two distinct solutions to the 4-queens puzzle as shown.
// Example 2:

// Input: n = 1
// Output: 1
 

// Constraints:

// 1 <= n <= 9


class Solution {
public:
    int totalNQueens(int n) {
        vector<int> cols(n, 0);
        vector<int> diag1(2 * n - 1, 0); // row - col + (n - 1)
        vector<int> diag2(2 * n - 1, 0); // row + col
        int count = 0;

        backtrack(0, n, cols, diag1, diag2, count);
        return count;
    }

private:
    void backtrack(int row, int n, vector<int>& cols, vector<int>& diag1,
                   vector<int>& diag2, int& count) {
        if (row == n) {
            count++;
            return;
        }

        for (int col = 0; col < n; col++) {
            int d1 = row - col + (n - 1);
            int d2 = row + col;

            if (cols[col] || diag1[d1] || diag2[d2]) {
                continue;
            }

            cols[col] = 1;
            diag1[d1] = 1;
            diag2[d2] = 1;

            backtrack(row + 1, n, cols, diag1, diag2, count);

            cols[col] = 0;
            diag1[d1] = 0;
            diag2[d2] = 0;
        }
    }
};
