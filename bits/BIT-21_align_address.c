/*
 * BIT-21: Align an address up / down, check pointer alignment
 *   align_up(x, a)   = (x + a - 1) & ~(a - 1)      a must be a power of 2
 *   align_down(x, a) = x & ~(a - 1)
 *   is_aligned(p, a) = ((uintptr_t)p & (a - 1)) == 0
 *   padding needed   = (-x) & (a - 1)
 * Do pointer math on uintptr_t, never on int (pointers can be 64-bit).
 * Uses: DMA buffers, cache-line alignment, flash page / sector boundaries,
 * stack alignment (8 bytes on ARM AAPCS), unaligned access faults on Cortex-M0.
 */
#include <stdio.h>
#include <stdint.h>

#define IS_POW2(a)          ((a) && !((a) & ((a) - 1)))
#define ALIGN_UP(x, a)      (((x) + ((a) - 1)) & ~((uintptr_t)(a) - 1))
#define ALIGN_DOWN(x, a)    ((x) & ~((uintptr_t)(a) - 1))
#define PAD_TO(x, a)        ((0u - (x)) & ((a) - 1))
#define IS_ALIGNED(p, a)    ((((uintptr_t)(p)) & ((uintptr_t)(a) - 1)) == 0)

static void *align_ptr_up(void *p, uintptr_t a) { return (void *)ALIGN_UP((uintptr_t)p, a); }

int main(void)
{
    uintptr_t addrs[] = {0x1000, 0x1001, 0x1007, 0x1008, 0x10FF};
    unsigned char buf[64];
    unsigned i;

    printf("   addr  up8    down8  pad8  up256\n");
    for (i = 0; i < sizeof addrs / sizeof addrs[0]; i++)
        printf(" 0x%04X 0x%04X 0x%04X %4u  0x%04X\n", (unsigned)addrs[i],
               (unsigned)ALIGN_UP(addrs[i], 8), (unsigned)ALIGN_DOWN(addrs[i], 8),
               (unsigned)PAD_TO(addrs[i], 8), (unsigned)ALIGN_UP(addrs[i], 256));

    /* flash: a 300-byte write starting at 0x08000F80 with 256-byte pages */
    {
        uint32_t start = 0x08000F80u, len = 300, page = 256;
        uint32_t first = (uint32_t)ALIGN_DOWN(start, page), last = (uint32_t)ALIGN_DOWN(start + len - 1, page);
        printf("flash write 0x%08X+%u touches pages 0x%08X..0x%08X (%u pages)\n",
               (unsigned)start, (unsigned)len, (unsigned)first, (unsigned)last, (unsigned)((last - first) / page + 1));
    }

    printf("IS_POW2(64)=%d IS_POW2(48)=%d\n", IS_POW2(64), IS_POW2(48));
    for (i = 0; i < 4; i++) {
        unsigned char *p = buf + i;
        printf("buf+%u: 4-byte aligned=%d, aligned-up pointer is aligned=%d\n", i,
               IS_ALIGNED(p, 4), IS_ALIGNED(align_ptr_up(p, 4), 4));
    }
    return 0;
}
