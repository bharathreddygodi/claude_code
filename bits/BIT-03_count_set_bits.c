/*
 * BIT-03: Count set bits (population count / Hamming weight)
 *  1) loop over every bit           O(32)
 *  2) Brian Kernighan: x &= x - 1   O(number of set bits)  <- the expected answer
 *  3) 4-bit lookup table            O(8)
 *  4) SWAR parallel bit count       O(1), no branches
 *  5) __builtin_popcount            single instruction on most CPUs
 */
#include <stdio.h>
#include <stdint.h>

int pop_naive(uint32_t x)
{
    int c = 0;
    while (x) { c += x & 1; x >>= 1; }
    return c;
}

int pop_kernighan(uint32_t x)
{
    int c = 0;
    while (x) { x &= x - 1; c++; }   /* clears the lowest set bit each time */
    return c;
}

int pop_table(uint32_t x)
{
    static const uint8_t nib[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
    int c = 0;
    while (x) { c += nib[x & 0xF]; x >>= 4; }
    return c;
}

int pop_swar(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555u);
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;
    return (int)((x * 0x01010101u) >> 24);
}

int main(void)
{
    uint32_t t[] = {0, 1, 7, 0x80000000u, 0xFFFFFFFFu, 0xA5A5A5A5u, 12345678};
    unsigned i;
    for (i = 0; i < sizeof t / sizeof t[0]; i++) {
        int a = pop_naive(t[i]), b = pop_kernighan(t[i]), c = pop_table(t[i]), d = pop_swar(t[i]);
        int e = __builtin_popcount(t[i]);
        printf("0x%08X -> %2d %2d %2d %2d %2d %s\n", (unsigned)t[i], a, b, c, d, e,
               (a == b && b == c && c == d && d == e) ? "PASS" : "FAIL");
    }
    return 0;
}
