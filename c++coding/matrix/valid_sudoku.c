// very difficult

#include <stdbool.h>

bool isValidSudoku(char** board, int boardSize, int* boardColSize) {
    int rows[9][9] = {0};
    int cols[9][9] = {0};
    int boxes[9][9] = {0};

    (void)boardSize;
    (void)boardColSize;

    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (board[r][c] == '.') {
                continue;
            }

            int digit = board[r][c] - '1';
            int box = (r / 3) * 3 + (c / 3);

            if (rows[r][digit] || cols[c][digit] || boxes[box][digit]) {
                return false;
            }

            rows[r][digit] = 1;
            cols[c][digit] = 1;
            boxes[box][digit] = 1;
        }
    }

    return true;
}
