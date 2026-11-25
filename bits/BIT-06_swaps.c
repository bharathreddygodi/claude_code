/*
 * BIT-06: Swap tricks
 *  - swap two variables with XOR (no temp) - and why it FAILS if a and b alias
 *  - swap nibbles of a byte
 *  - swap odd and even bits
 *  - swap bits at positions i and j
 */
#include <stdio.h>
#include <stdint.h>

void xor_swap(int *a, int *b)
{
    if (a == b) return;          /* without this, *a ^= *a makes it 0 */
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

uint8_t swap_nibbles(uint8_t x) { return (uint8_t)((x << 4) | (x >> 4)); }

uint32_t swap_odd_even(uint32_t x)
{
    return ((x & 0xAAAAAAAAu) >> 1) | ((x & 0x55555555u) << 1);
}

uint32_t swap_bits(uint32_t x, int i, int j)
{
    if (((x >> i) & 1) != ((x >> j) & 1))   /* only act if the bits differ */
        x ^= (1u << i) | (1u << j);
    return x;
}

int main(void)
{
    int a = 5, b = 9;
    xor_swap(&a, &b);
    printf("xor_swap: a=%d b=%d %s\n", a, b, (a == 9 && b == 5) ? "PASS" : "FAIL");
    xor_swap(&a, &a);
    printf("xor_swap same var: a=%d %s\n", a, a == 9 ? "PASS" : "FAIL");

    printf("swap_nibbles(0xAB) = 0x%02X\n", swap_nibbles(0xAB));
    printf("swap_odd_even(0x17) = 0x%02X (expect 0x2B)\n", (unsigned)swap_odd_even(0x17));
    printf("swap_bits(0x01, 0, 4) = 0x%02X (expect 0x10)\n", (unsigned)swap_bits(0x01, 0, 4));
    return 0;
}
