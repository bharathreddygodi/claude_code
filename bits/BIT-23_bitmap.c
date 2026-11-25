/*
 * BIT-23: Bitmap / bit array - track N flags using N/8 bytes
 * Used for: free-block maps in allocators/filesystems, RTOS ready lists,
 * "find duplicates in a large range with little memory", Bloom filters.
 * word index = n / 32 (n >> 5), bit index = n % 32 (n & 31).
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define NBITS 1000u
#define WORDS ((NBITS + 31u) / 32u)

static uint32_t bm[WORDS];

static void bm_set(unsigned n)   { bm[n >> 5] |=  (1u << (n & 31)); }
static void bm_clear(unsigned n) { bm[n >> 5] &= ~(1u << (n & 31)); }
static int  bm_test(unsigned n)  { return (bm[n >> 5] >> (n & 31)) & 1u; }

static int bm_first_zero(void)  /* first free slot, e.g. block allocator */
{
    unsigned w;
    for (w = 0; w < WORDS; w++)
        if (bm[w] != 0xFFFFFFFFu) {
            int pos = (int)(w * 32u) + __builtin_ctz(~bm[w]);
            return pos < (int)NBITS ? pos : -1;
        }
    return -1;
}

int main(void)
{
    int nums[] = {5, 999, 42, 5, 700, 42, 3};
    unsigned i;

    memset(bm, 0, sizeof bm);
    printf("bitmap for %u flags uses %u bytes\n", NBITS, (unsigned)sizeof bm);
    printf("duplicates: ");
    for (i = 0; i < sizeof nums / sizeof nums[0]; i++) {
        if (bm_test((unsigned)nums[i])) printf("%d ", nums[i]);
        bm_set((unsigned)nums[i]);
    }
    printf("\n");

    memset(bm, 0, sizeof bm);
    for (i = 0; i < 40; i++) bm_set(i);
    bm_clear(17);
    printf("first free slot = %d (expect 17)\n", bm_first_zero());
    return 0;
}
