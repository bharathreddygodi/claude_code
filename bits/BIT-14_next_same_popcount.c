/*
 * BIT-14: Next higher number with the same number of set bits (Gosper's hack)
 *   x = 0b0110_0  ->  0b1000_1  (6<<1=12 -> 17)
 * Steps:
 *   c = x & -x              lowest set bit
 *   r = x + c               ripple: the rightmost block of 1s becomes a single higher 1
 *   ones = ((r ^ x) >> 2) / c   the leftover 1s, moved to the bottom
 *   next = r | ones
 * Also shown: the slow obvious method (count up until popcount matches) to verify,
 * and the classic use: iterate over all k-bit subsets of n items in increasing order.
 */
#include <stdio.h>
#include <stdint.h>

uint32_t next_same_popcount(uint32_t x)   /* x != 0; returns 0 on overflow */
{
    uint32_t c = x & (0u - x);
    uint32_t r = x + c;
    if (r == 0) return 0;                  /* the 1s were already at the top */
    return r | (((r ^ x) >> 2) / c);
}

static uint32_t next_slow(uint32_t x)
{
    int p = __builtin_popcount(x);
    uint32_t y = x + 1;
    while (y && __builtin_popcount(y) != p) y++;
    return y;
}

static void print_bin(uint32_t x, int w)
{
    int i;
    for (i = w - 1; i >= 0; i--) putchar(((x >> i) & 1) ? '1' : '0');
}

int main(void)
{
    uint32_t t[] = {1, 6, 7, 12, 0x2C, 0x0F0, 0x7FFF0000u};
    uint32_t s;
    unsigned i;
    int ok = 1, count = 0;

    for (i = 0; i < sizeof t / sizeof t[0]; i++) {
        uint32_t a = next_same_popcount(t[i]), b = next_slow(t[i]);
        ok &= (a == b);
        printf("%10u -> %10u %s\n", (unsigned)t[i], (unsigned)a, a == b ? "PASS" : "FAIL");
    }
    printf("0xF0000000 (top bits full) -> %u (no larger value)\n", (unsigned)next_same_popcount(0xF0000000u));

    printf("all 3-of-5 subsets in order:\n ");
    for (s = 0x7; s < (1u << 5); s = next_same_popcount(s)) {   /* start with the lowest 3 bits */
        print_bin(s, 5);
        putchar(' ');
        count++;
    }
    printf("\n count = %d (expect C(5,3) = 10) %s\n", count, (count == 10 && ok) ? "PASS" : "FAIL");
    return 0;
}
