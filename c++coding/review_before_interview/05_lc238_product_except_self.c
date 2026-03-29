// LC 238: Product of Array Except Self
// Return array where result[i] = product of all elements except nums[i].
// No division. O(n) time.
// Hint: two passes — prefix products left-to-right, then suffix products right-to-left.

#include <stdio.h>

void productExceptSelf(int* nums, int n, int* result) {
    // Pass 1: result[i] = product of everything LEFT of i
    int prefix = 1;
    for (int i = 0; i < n; i++) {
        result[i] = prefix;    // store prefix so far (excludes nums[i])
        prefix *= nums[i];     // then include nums[i] for next iteration
    }

    // Pass 2: multiply result[i] by product of everything RIGHT of i
    int suffix = 1;
    for (int i = n - 1; i >= 0; i--) {
        result[i] *= suffix;   // multiply by suffix so far (excludes nums[i])
        suffix *= nums[i];     // then include nums[i] for next iteration
    }
}

int main() {
    int nums[] = {1, 2, 3, 4};
    int result[4];
    productExceptSelf(nums, 4, result);
    printf("Result: ");
    for (int i = 0; i < 4; i++) printf("%d ", result[i]);
    printf("\n");  // expected: 24 12 8 6
    return 0;
}
