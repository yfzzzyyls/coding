// LC 33: Search in Rotated Sorted Array
// Sorted array rotated at some pivot. Find target in O(log n).
// Example: [4,5,6,7,0,1,2], target=0 -> return 4
// Hint: binary search — one half is always sorted, check if target is in that half.

#include <stdio.h>

int search(int* nums, int n, int target) {
    // TODO: your solution here
}

int main() {
    int nums[] = {4, 5, 6, 7, 0, 1, 2};
    printf("Index of 0: %d\n", search(nums, 7, 0));  // expected: 4
    printf("Index of 3: %d\n", search(nums, 7, 3));  // expected: -1
    printf("Index of 5: %d\n", search(nums, 7, 5));  // expected: 1
    return 0;
}
