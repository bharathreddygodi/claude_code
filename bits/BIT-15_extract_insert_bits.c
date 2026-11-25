/*
 * BIT-15: Extract n bits from position p, insert bits of M into N between i..j
 *  extract: (x >> p) & ((1 << n) - 1)
 *  insert (CTCI classic): clear bits i..j in N, then OR in (M << i)
 *  careful: (1 << 32) is UB -> handle full-width masks separately.
 */
#include <stdio.h>
#include <stdint.h>

static uint32_t mask_n(unsigned n) { return n >= 32 ? 0xFFFFFFFFu : ((1u << n) - 1u); }

uint32_t extract_bits(uint32_t x, unsigned p, unsigned n)
{
    return (x >> p) & mask_n(n);
}

uint32_t insert_bits(uint32_t N, uint32_t M, unsigned i, unsigned j)   /* i <= j */
{
    uint32_t m = mask_n(j - i + 1) << i;
    return (N & ~m) | ((M << i) & m);
}

int main(void)
{
    uint32_t x = 0xABCD1234u;
    printf("extract nibble at 8 of 0x%08X = 0x%X (expect 0x2)\n", (unsigned)x, (unsigned)extract_bits(x, 8, 4));
    printf("extract byte at 24            = 0x%X (expect 0xAB)\n", (unsigned)extract_bits(x, 24, 8));
    printf("extract all 32                = 0x%08X\n", (unsigned)extract_bits(x, 0, 32));

    /* N = 10000000000, M = 10011, i=2, j=6 -> 10001001100 */
    uint32_t r = insert_bits(0x400, 0x13, 2, 6);
    printf("insert_bits = 0x%X (expect 0x44C) %s\n", (unsigned)r, r == 0x44C ? "PASS" : "FAIL");
    return 0;
}
