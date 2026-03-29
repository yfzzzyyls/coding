// LC 190: Reverse Bits
// Reverse all 32 bits of a given unsigned integer.
// Example: 0b1011 (4-bit) -> 0b1101

#include <stdio.h>
#include <stdint.h>

uint32_t reverseBits(uint32_t n) {
    uint32_t result = 0;
    uint32_t bit = 0;
    uint32_t bit_mask = 1;
    for (int i = 0; i < 32; i++){
        bit = n & bit_mask;
        result = (result << 1) | bit;
        n = n >> 1;
    }
    return result;
}

int main() {
    // 00000010100101000001111010011100 -> 00111001011110000010100101000000
    printf("%u\n", reverseBits(43261596));   // expected: 964176192
    printf("%u\n", reverseBits(4294967293)); // expected: 3221225471
    return 0;
}
