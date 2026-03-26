void dfs(char** grid, int row, int col, int rows, int cols) {
    if (row < 0 || row >= rows || col < 0 || col >= cols || grid[row][col] == '0') {
        return;
    }

    grid[row][col] = '0';

    dfs(grid, row + 1, col, rows, cols);
    dfs(grid, row - 1, col, rows, cols);
    dfs(grid, row, col + 1, rows, cols);
    dfs(grid, row, col - 1, rows, cols);
}

int numIslands(char** grid, int gridSize, int* gridColSize) {
    int rows = gridSize;
    int cols = gridColSize[0];
    int count = 0;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid[r][c] == '1') {
                count++;
                dfs(grid, r, c, rows, cols);
            }
        }
    }

    return count;
}
