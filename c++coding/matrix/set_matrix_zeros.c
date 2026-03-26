// also very representative

void setZeroes(int** matrix, int matrixSize, int* matrixColSize) {
    int cols = matrixColSize[0];
    int rows_to_zero[matrixSize];
    int cols_to_zero[cols];

    memset(rows_to_zero, 0, sizeof(rows_to_zero));
    memset(cols_to_zero, 0, sizeof(cols_to_zero));


    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] == 0) {
                rows_to_zero[i] = 1;
                cols_to_zero[j] = 1;
            }
        }
    }

    for (int i = 0; i < matrixSize; i++) {
        for (int j = 0; j < cols; j++) {
            if (rows_to_zero[i] || cols_to_zero[j]) {
                matrix[i][j] = 0;
            }
        }
    }
}
