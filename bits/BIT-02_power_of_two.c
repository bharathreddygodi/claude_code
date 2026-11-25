/*
 * BIT-02: Is x a power of 2? Next power of 2? Round up to a multiple of 2^k?
 * Power of 2 has exactly one bit set -> x & (x - 1) == 0 (and x != 0!).
 * Next power of 2: smear the highest bit down, then add 1.
 * Used for: buffer sizes, alignment, hash table sizes, ring buffer masks.
 */
#include <stdio.h>
#include <stdint.h>

int is_pow2(uint32_t x) { return x && !(x & (x - 1)); }

uint32_t next_pow2(uint32_t x)          /* smallest power of 2 >= x (x>0) */
{
    if (x == 0) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

/* align must be a power of 2 */
#define ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define MOD_POW2(x, a)   ((x) & ((a) - 1))   /* x % a without division */

int main(void)
{
    uint32_t t[] = {0, 1, 2, 3, 16, 17, 100, 1024, 0x80000000u};
    unsigned i;
    for (i = 0; i < sizeof t / sizeof t[0]; i++)
        printf("%10u: is_pow2=%d next_pow2=%u\n", (unsigned)t[i], is_pow2(t[i]), (unsigned)next_pow2(t[i]));

    printf("ALIGN_UP(13, 8)=%u ALIGN_DOWN(13, 8)=%u 13%%8=%u\n",
           ALIGN_UP(13u, 8u), ALIGN_DOWN(13u, 8u), MOD_POW2(13u, 8u));
    return 0;
}
