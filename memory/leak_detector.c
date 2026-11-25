/*
 * Memory leak / double-free detector with malloc/free wrappers
 * - Macros capture __FILE__ and __LINE__ of every allocation.
 * - Each allocation is tracked in a list; report what is still alive at the end.
 * - SAFE_FREE sets the pointer to NULL to prevent dangling pointers / double free.
 * Interview points: dangling pointer, double free, use-after-free, leak,
 * tools: Valgrind, AddressSanitizer (-fsanitize=address), guard bytes/canaries.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct rec {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    struct rec *next;
} rec_t;

static rec_t *records;
static size_t bytes_in_use;

void *dbg_malloc(size_t size, const char *file, int line)
{
    void *p = malloc(size);
    rec_t *r = malloc(sizeof *r);
    if (!p || !r) {
        free(p);
        free(r);
        return NULL;
    }
    r->ptr = p; r->size = size; r->file = file; r->line = line;
    r->next = records;
    records = r;
    bytes_in_use += size;
    return p;
}

void dbg_free(void *p, const char *file, int line)
{
    rec_t **pp;
    if (!p)
        return;
    for (pp = &records; *pp; pp = &(*pp)->next) {
        if ((*pp)->ptr == p) {
            rec_t *dead = *pp;
            *pp = dead->next;
            bytes_in_use -= dead->size;
            free(dead);
            free(p);
            return;
        }
    }
    printf("  ERROR: invalid/double free of %p at %s:%d\n", p, file, line);
}

void dbg_report(void)
{
    rec_t *r;
    printf("---- leak report: %u bytes still allocated ----\n", (unsigned)bytes_in_use);
    for (r = records; r; r = r->next)
        printf("  LEAK %u bytes at %p, allocated %s:%d\n", (unsigned)r->size, r->ptr, r->file, r->line);
}

#define MALLOC(n)    dbg_malloc((n), __FILE__, __LINE__)
#define FREE(p)      dbg_free((p), __FILE__, __LINE__)
#define SAFE_FREE(p) do { FREE(p); (p) = NULL; } while (0)

int main(void)
{
    char *a = MALLOC(32);
    int *b = MALLOC(10 * sizeof *b);
    char *c = MALLOC(64);              /* leaked on purpose */
    char *d;

    strcpy(a, "tracked");
    FREE(a);
    FREE(a);                           /* double free -> detected */

    SAFE_FREE(b);
    SAFE_FREE(b);                      /* harmless: b is NULL now */
    printf("b after SAFE_FREE = %p\n", (void *)b);

    d = MALLOC(16);                    /* leaked on purpose */
    (void)c; (void)d;
    dbg_report();
    return 0;
}
