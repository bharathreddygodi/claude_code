/*
 * BIT-10: Count set bits for every number 0..n  (LeetCode "Counting Bits")
 * Naive: popcount each number -> O(n * 32).
 * DP O(n):  bits[i] = bits[i >> 1] + (i & 1)      i without its last bit, plus that bit
 *   or      bits[i] = bits[i & (i - 1)] + 1       i without its lowest set bit, plus 1
 * Embedded angle: this IS how you build a popcount lookup table at startup
 * (256 entries for a byte LUT) instead of hard-coding it.
 */
#include <stdio.h>
#include <stdint.h>

void count_bits_shift(uint8_t *bits, unsigned n)
{
    unsigned i;
    bits[0] = 0;
    for (i = 1; i <= n; i++) bits[i] = (uint8_t)(bits[i >> 1] + (i & 1));
}

void count_bits_lowbit(uint8_t *bits, unsigned n)
{
    unsigned i;
    bits[0] = 0;
    for (i = 1; i <= n; i++) bits[i] = (uint8_t)(bits[i & (i - 1)] + 1);
}

int main(void)
{
    uint8_t a[256], b[256];
    unsigned i;
    int ok = 1;

    count_bits_shift(a, 255);
    count_bits_lowbit(b, 255);
    for (i = 0; i <= 255; i++)
        ok &= (a[i] == b[i]) && (a[i] == (uint8_t)__builtin_popcount(i));

    printf("bits[0..16] :");
    for (i = 0; i <= 16; i++) printf(" %u", a[i]);
    printf("\nboth DP methods match __builtin_popcount for 0..255 -> %s\n", ok ? "PASS" : "FAIL");

    /* use the table as a byte LUT to popcount a 32-bit value */
    {
        uint32_t x = 0xDEADBEEFu;
        unsigned c = a[x & 0xFF] + a[(x >> 8) & 0xFF] + a[(x >> 16) & 0xFF] + a[x >> 24];
        printf("LUT popcount(0x%08X) = %u (builtin %d)\n", (unsigned)x, c, __builtin_popcount(x));
    }
    return 0;
}
