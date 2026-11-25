/*
 * MEM-06: Write your own malloc()/free() on top of a static heap array
 * - First-fit or best-fit search over a linked list of block headers.
 *     first-fit: take the first block big enough - fast, fragments the front of the heap
 *     best-fit : take the smallest block big enough - keeps big holes intact, but scans
 *                the whole list and tends to leave tiny unusable slivers
 * - Split a block if it is much bigger than requested.
 * - On free, coalesce with the next and previous free neighbours to fight fragmentation.
 * Interview points: block header (size + free flag), alignment of returned memory,
 * internal vs external fragmentation, first-fit vs best-fit, double free detection.
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define HEAP_SIZE 1024u
#define ALIGN     8u
#define ALIGN_UP(x) (((x) + (ALIGN - 1)) & ~(size_t)(ALIGN - 1))

typedef struct header {
    size_t size;               /* payload size in bytes */
    int is_free;
    struct header *next;
    struct header *prev;
} header_t;

#define HDR_SIZE ALIGN_UP(sizeof(header_t))

static union { uint8_t bytes[HEAP_SIZE]; uint64_t force_align; } heap;
static header_t *head;
static int use_best_fit;           /* 0 = first-fit, 1 = best-fit */

static void heap_init(void)
{
    head = (header_t *)heap.bytes;
    head->size = HEAP_SIZE - HDR_SIZE;
    head->is_free = 1;
    head->next = head->prev = NULL;
}

void *my_malloc(size_t size)
{
    header_t *b, *pick = NULL;
    if (size == 0)
        return NULL;
    if (!head)
        heap_init();
    size = ALIGN_UP(size);

    for (b = head; b; b = b->next) {
        if (!b->is_free || b->size < size)
            continue;
        if (!use_best_fit) { pick = b; break; }    /* first fit: stop at the first match */
        if (!pick || b->size < pick->size) pick = b;   /* best fit: keep the tightest */
        if (pick->size == size) break;             /* perfect fit, can't do better */
    }
    if (!pick)
        return NULL;                               /* out of memory */

    b = pick;
    if (b->size >= size + HDR_SIZE + ALIGN) {      /* split leftover into a new free block */
        header_t *n = (header_t *)((uint8_t *)b + HDR_SIZE + size);
        n->size = b->size - size - HDR_SIZE;
        n->is_free = 1;
        n->next = b->next;
        n->prev = b;
        if (b->next)
            b->next->prev = n;
        b->next = n;
        b->size = size;
    }
    b->is_free = 0;
    return (uint8_t *)b + HDR_SIZE;
}

void my_free(void *p)
{
    header_t *b;
    if (!p)
        return;
    b = (header_t *)((uint8_t *)p - HDR_SIZE);
    if (b->is_free) {
        printf("  [heap] double free detected at %p\n", p);
        return;
    }
    b->is_free = 1;

    if (b->next && b->next->is_free) {             /* merge with next */
        b->size += HDR_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next)
            b->next->prev = b;
    }
    if (b->prev && b->prev->is_free) {             /* merge with previous */
        b->prev->size += HDR_SIZE + b->size;
        b->prev->next = b->next;
        if (b->next)
            b->next->prev = b->prev;
    }
}

static void heap_dump(const char *tag)
{
    header_t *b;
    printf("%-18s:", tag);
    for (b = head; b; b = b->next)
        printf(" [%s %u]", b->is_free ? "F" : "U", (unsigned)b->size);
    printf("\n");
}

int main(void)
{
    void *a = my_malloc(100);
    void *b = my_malloc(200);
    void *c = my_malloc(50);
    heap_dump("after a,b,c");

    my_free(b);
    heap_dump("free b");
    my_free(a);
    heap_dump("free a (merge)");

    void *d = my_malloc(250);                      /* fits in merged a+b hole */
    printf("d reused hole -> %s\n", d == a ? "PASS" : "FAIL");
    heap_dump("alloc d=250");

    my_free(d);
    my_free(c);
    heap_dump("free all");
    my_free(c);                                    /* double free */
    printf("huge alloc -> %s\n", my_malloc(5000) == NULL ? "NULL (PASS)" : "FAIL");

    printf("\nfirst-fit vs best-fit (holes of 200 and 64, then alloc 50 and 190):\n");
    for (use_best_fit = 0; use_best_fit <= 1; use_best_fit++) {
        header_t *t;
        void *h1, *h2, *small, *big;
        head = NULL;                               /* reset the heap */
        h1 = my_malloc(200); my_malloc(16);
        h2 = my_malloc(64);  my_malloc(16);
        for (t = head; t->next; t = t->next) { }   /* fill the rest so only the holes remain */
        my_malloc(t->size);
        my_free(h1);
        my_free(h2);
        small = my_malloc(50);
        big = my_malloc(190);
        printf("  %s: 50 went into the %s hole, then 190 -> %s\n",
               use_best_fit ? "best-fit " : "first-fit", small == h1 ? "200" : "64",
               big ? "fits (big hole kept intact)" : "NULL (big hole was split up)");
    }
    return 0;
}
