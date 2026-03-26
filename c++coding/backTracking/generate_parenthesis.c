#include <stdlib.h>
#include <string.h>

static void backtrack(int n, int openUsed, int closeUsed, int pos, char* current,
                      char*** result, int* returnSize, int* capacity) {
    if (pos == 2 * n) {
        char* s = (char*)malloc((2 * n + 1) * sizeof(char));
        memcpy(s, current, (2 * n + 1) * sizeof(char));

        if (*returnSize == *capacity) {
            *capacity *= 2;
            *result = (char**)realloc(*result, (*capacity) * sizeof(char*));
        }

        (*result)[(*returnSize)++] = s;
        return;
    }

    if (openUsed < n) {
        current[pos] = '(';
        backtrack(n, openUsed + 1, closeUsed, pos + 1, current,
                  result, returnSize, capacity);
    }

    if (closeUsed < openUsed) {
        current[pos] = ')';
        backtrack(n, openUsed, closeUsed + 1, pos + 1, current,
                  result, returnSize, capacity);
    }
}

char** generateParenthesis(int n, int* returnSize) {
    int capacity = 16;
    char** result = (char**)malloc(capacity * sizeof(char*));
    char* current = (char*)malloc((2 * n + 1) * sizeof(char));

    *returnSize = 0;
    current[2 * n] = '\0';

    backtrack(n, 0, 0, 0, current, &result, returnSize, &capacity);

    free(current);
    return result;
}
