#include <stdio.h>
#include <stdlib.h>

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
char** summaryRanges(int* nums, int numsSize, int* returnSize) {
    char** result = (char**)malloc(numsSize * sizeof(char*));
    int count = 0;

    for (int i = 0; i < numsSize; i++) {
        int start = nums[i];

        while (i + 1 < numsSize && nums[i + 1] == nums[i] + 1) {
            i++;
        }

        int end = nums[i];
        char* range = (char*)malloc(25 * sizeof(char));

        if (start == end) {
            sprintf(range, "%d", start);
        } else {
            sprintf(range, "%d->%d", start, end);
        }

        result[count] = range;
        count++;
    }

    *returnSize = count;
    return result;
}
