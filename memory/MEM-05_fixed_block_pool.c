/*
 * MEM-05: Fixed-size block memory pool allocator (classic embedded question)
 * - Static array split into N equal blocks; free blocks form a linked list
 *   stored INSIDE the free blocks themselves (zero overhead).
 * - alloc and free are O(1), deterministic, no fragmentation -> RTOS friendly.
 * Interview points: why not malloc in embedded (non-deterministic, fragmentation),
 * block size >= sizeof(void*), alignment, validating the pointer on free,
 * make it thread/ISR safe by disabling interrupts or using a mutex around alloc/free.
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define BLOCK_SIZE   32u
#define BLOCK_COUNT  8u

typedef union block {
    union block *next;                 /* used while the block is free */
    uint8_t data[BLOCK_SIZE];          /* used while the block is allocated */
} block_t;

static block_t pool[BLOCK_COUNT];
static block_t *free_list;
static unsigned free_count;

void pool_init(void)
{
    unsigned i;
    for (i = 0; i < BLOCK_COUNT - 1; i++)
        pool[i].next = &pool[i + 1];
    pool[BLOCK_COUNT - 1].next = NULL;
    free_list = &pool[0];
    free_count = BLOCK_COUNT;
}

void *pool_alloc(void)
{
    /* ENTER_CRITICAL(); */
    block_t *b = free_list;
    if (b) {
        free_list = b->next;
        free_count--;
    }
    /* EXIT_CRITICAL(); */
    return b;
}

int pool_free(void *p)
{
    block_t *b = (block_t *)p;
    if (!p)
        return 0;
    /* reject pointers that are outside the pool or not on a block boundary */
    if (b < &pool[0] || b > &pool[BLOCK_COUNT - 1] ||
        ((uintptr_t)b - (uintptr_t)pool) % sizeof(block_t) != 0)
        return -1;
    /* ENTER_CRITICAL(); */
    b->next = free_list;
    free_list = b;
    free_count++;
    /* EXIT_CRITICAL(); */
    return 0;
}

int main(void)
{
    void *p[BLOCK_COUNT + 1];
    unsigned i;
    int bad = 0;

    pool_init();
    for (i = 0; i < BLOCK_COUNT; i++)
        p[i] = pool_alloc();
    p[BLOCK_COUNT] = pool_alloc();
    printf("allocated %u blocks, free=%u, extra alloc -> %s\n",
           BLOCK_COUNT, free_count, p[BLOCK_COUNT] == NULL ? "NULL (PASS)" : "FAIL");

    pool_free(p[3]);
    pool_free(p[5]);
    printf("after 2 frees free=%u -> %s\n", free_count, free_count == 2 ? "PASS" : "FAIL");
    printf("next alloc reuses last freed block -> %s\n", pool_alloc() == p[5] ? "PASS" : "FAIL");

    printf("free(invalid ptr) -> %s\n", pool_free((uint8_t *)p[0] + 1) == -1 ? "rejected (PASS)" : "FAIL");
    printf("free(&bad)        -> %s\n", pool_free(&bad) == -1 ? "rejected (PASS)" : "FAIL");
    return 0;
}
