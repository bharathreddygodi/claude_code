/*
 * BIT-22: Bitmap allocator - find first free, allocate, free (single and contiguous runs)
 * One bit per fixed-size block: 1 = used, 0 = free. Used for flash sectors, DMA
 * descriptors, file-system blocks, RTOS object pools.
 *   find first free: skip words that are all 1s, then ctz(~word)
 *   allocate n contiguous blocks: scan for a run of n zero bits
 *   free: clear bits; validate the range (catch double free)
 * Compare with MEM-05 (free-list pool): bitmap needs 1 bit per block and can do
 * contiguous runs; free list is O(1) but can't.
 */
#include <stdio.h>
#include <stdint.h>

#define NBLOCKS 80u
#define NWORDS  ((NBLOCKS + 31u) / 32u)

static uint32_t map[NWORDS];

static int  test_bit(unsigned i)  { return (map[i >> 5] >> (i & 31)) & 1u; }
static void set_bit(unsigned i)   { map[i >> 5] |=  (1u << (i & 31)); }
static void clear_bit(unsigned i) { map[i >> 5] &= ~(1u << (i & 31)); }

int find_first_free(void)
{
    unsigned w;
    for (w = 0; w < NWORDS; w++) {
        if (map[w] != 0xFFFFFFFFu) {
            unsigned i = w * 32u + (unsigned)__builtin_ctz(~map[w]);
            return i < NBLOCKS ? (int)i : -1;
        }
    }
    return -1;
}

int alloc_one(void)
{
    int i = find_first_free();
    if (i >= 0) set_bit((unsigned)i);
    return i;
}

int alloc_run(unsigned n)                 /* first-fit run of n free blocks */
{
    unsigned i, run = 0;
    for (i = 0; i < NBLOCKS; i++) {
        run = test_bit(i) ? 0 : run + 1;
        if (run == n) {
            unsigned start = i + 1 - n, k;
            for (k = start; k <= i; k++) set_bit(k);
            return (int)start;
        }
    }
    return -1;
}

int free_run(unsigned start, unsigned n)  /* -1 on bad range or double free */
{
    unsigned k;
    if (start + n > NBLOCKS) return -1;
    for (k = start; k < start + n; k++) if (!test_bit(k)) return -1;
    for (k = start; k < start + n; k++) clear_bit(k);
    return 0;
}

static unsigned used_count(void)
{
    unsigned w, c = 0;
    for (w = 0; w < NWORDS; w++) c += (unsigned)__builtin_popcount(map[w]);
    return c;
}

static void show(const char *tag)
{
    unsigned i;
    printf("%-24s ", tag);
    for (i = 0; i < 40; i++) putchar(test_bit(i) ? '#' : '.');
    printf(" used=%u\n", used_count());
}

int main(void)
{
    int a, b, c, d, i;

    for (i = 0; i < 5; i++) alloc_one();
    show("5 single allocs");
    a = alloc_run(8);   show("run of 8");
    b = alloc_run(4);   show("run of 4");
    free_run(2, 1);     show("free block 2");
    free_run((unsigned)a, 8); show("free run of 8");
    c = alloc_one();    printf("next single -> block %d (first free, expect 2)\n", c);
    d = alloc_run(6);   show("run of 6 (fits in hole)");
    printf("run of 6 at %d, run of 4 was at %d\n", d, b);
    printf("free block %d -> %d (ok)\n", c, free_run((unsigned)c, 1));
    printf("free block %d again -> %d (double free caught)\n", c, free_run((unsigned)c, 1));
    printf("free 78..82 -> %d (out of range)\n", free_run(78, 5));
    printf("run of 100 -> %d (too big)\n", alloc_run(100));
    return 0;
}
