// LC 191: Number of 1 Bits (Hamming Weight)
// Given an unsigned integer, return the number of 1 bits.
// Hint: n & (n-1) clears the lowest set bit.

#include <stdio.h>

int hammingWeight(unsigned int n) {
    int num = 0;
    while (n != 0){
        n = n & (n-1);
        num++;
    }
    return num;
}

int main() {
    printf("1 bits in 11 (0b1011): %d\n", hammingWeight(11));   // expected: 3
    printf("1 bits in 128 (0b10000000): %d\n", hammingWeight(128)); // expected: 1
    printf("1 bits in 255 (0b11111111): %d\n", hammingWeight(255)); // expected: 8
    return 0;
}
