// LC 136: Single Number
// Given an integer array where every element appears twice except one,
// find the single element.
// Constraint: O(n) time, O(1) space.

#include <stdio.h>

int singleNumber(int* nums, int n) {
    int ele = 0;
    for (int i = 0; i < n; i++){
        ele = ele ^ nums[i];
    }
    return ele;
}

int main() {
    int nums[] = {4, 1, 2, 1, 2};
    int n = sizeof(nums) / sizeof(nums[0]);
    printf("Single number: %d\n", singleNumber(nums, n));  // expected: 4
    return 0;
}
