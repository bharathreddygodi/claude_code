/*
 * MEM-10: Buddy allocator (used by the Linux page allocator, many RTOS heaps)
 * Heap of 2^MAX_ORDER bytes; every block is 2^k bytes and starts at a multiple of 2^k.
 *   alloc(n): round n up to a power of 2 (order k); take the smallest free block of
 *             order >= k and split it in halves until it is order k. Each split puts the
 *             upper half ("buddy") on the free list of the lower order.
 *   free(p) : the buddy of a block at offset off with order k is at  off ^ (1 << k).
 *             If the buddy is free AND has the same order, merge into one block of order
 *             k+1 (at offset off & ~(1 << k)) and repeat.
 * Pros: O(log n) alloc/free, cheap coalescing, no external fragmentation between buddies.
 * Cons: internal fragmentation (a 65-byte request uses 128 bytes).
 * Metadata: one byte of order + a free flag per minimum-size block, kept outside the heap.
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define MIN_ORDER 4                       /* 16-byte minimum block */
#define MAX_ORDER 10                      /* 1024-byte heap */
#define HEAP_SIZE (1u << MAX_ORDER)
#define NMIN      (HEAP_SIZE >> MIN_ORDER)

typedef struct fblk { struct fblk *next; } fblk_t;   /* free-list node lives in the free block */

static union { uint8_t bytes[HEAP_SIZE]; uint64_t align; } heap;
static fblk_t *free_list[MAX_ORDER + 1];
static uint8_t blk_order[NMIN];           /* order of the block that starts at this min-block */
static uint8_t blk_free[NMIN];            /* 1 if that block is on a free list */

static size_t off_of(const void *p) { return (size_t)((const uint8_t *)p - heap.bytes); }
static void  *ptr_at(size_t off)      { return heap.bytes + off; }
static size_t idx_of(size_t off)      { return off >> MIN_ORDER; }

static void push(int k, size_t off)
{
    fblk_t *b = ptr_at(off);
    b->next = free_list[k];
    free_list[k] = b;
    blk_order[idx_of(off)] = (uint8_t)k;
    blk_free[idx_of(off)] = 1;
}

static void unlink_block(int k, size_t off)
{
    fblk_t **pp = &free_list[k];
    while (*pp != ptr_at(off)) pp = &(*pp)->next;
    *pp = (*pp)->next;
    blk_free[idx_of(off)] = 0;
}

void buddy_init(void)
{
    int k;
    for (k = 0; k <= MAX_ORDER; k++) free_list[k] = NULL;
    push(MAX_ORDER, 0);                   /* one big free block */
}

static int order_for(size_t n)
{
    int k = MIN_ORDER;
    while ((1u << k) < n) k++;
    return k;
}

void *buddy_alloc(size_t n)
{
    int k, j;
    size_t off;
    if (n == 0 || n > HEAP_SIZE) return NULL;
    k = order_for(n);
    for (j = k; j <= MAX_ORDER && !free_list[j]; j++) { }
    if (j > MAX_ORDER) return NULL;       /* out of memory */

    off = off_of(free_list[j]);
    unlink_block(j, off);
    while (j > k) {                       /* split, keep the lower half */
        j--;
        push(j, off + (1u << j));         /* upper half = buddy, goes free */
    }
    blk_order[idx_of(off)] = (uint8_t)k;
    return ptr_at(off);
}

void buddy_free(void *p)
{
    size_t off;
    int k;
    if (!p) return;
    off = off_of(p);
    k = blk_order[idx_of(off)];
    if (blk_free[idx_of(off)]) { printf("  double free at offset %u ignored\n", (unsigned)off); return; }

    while (k < MAX_ORDER) {
        size_t buddy = off ^ (1u << k);
        if (!blk_free[idx_of(buddy)] || blk_order[idx_of(buddy)] != k) break;
        unlink_block(k, buddy);           /* merge */
        off &= ~(size_t)(1u << k);        /* merged block starts at the lower address */
        k++;
    }
    push(k, off);
}

static void dump(const char *tag)
{
    int k;
    printf("%-26s free:", tag);
    for (k = MIN_ORDER; k <= MAX_ORDER; k++) {
        fblk_t *b;
        for (b = free_list[k]; b; b = b->next) printf(" [%u@%u]", 1u << k, (unsigned)off_of(b));
    }
    printf("\n");
}

int main(void)
{
    void *a, *b, *c, *d;

    buddy_init();
    dump("init");
    a = buddy_alloc(100);  dump("alloc 100 -> 128");
    b = buddy_alloc(40);   dump("alloc 40 -> 64");
    c = buddy_alloc(200);  dump("alloc 200 -> 256");
    d = buddy_alloc(16);   dump("alloc 16");
    printf("a@%u b@%u c@%u d@%u\n", (unsigned)off_of(a), (unsigned)off_of(b), (unsigned)off_of(c), (unsigned)off_of(d));
    printf("buddy of a (off 0, 128 B) = 0 ^ 128 = %u\n", 0u ^ 128u);

    buddy_free(b);         dump("free b");
    buddy_free(d);         dump("free d (merges up)");
    buddy_free(a);         dump("free a");
    buddy_free(a);
    buddy_free(c);         dump("free c (back to 1 block)");
    printf("whole heap free again -> %s\n", (free_list[MAX_ORDER] && !free_list[MAX_ORDER]->next) ? "PASS" : "FAIL");
    printf("alloc 2000 -> %s\n", buddy_alloc(2000) ? "FAIL" : "NULL (PASS)");
    return 0;
}
