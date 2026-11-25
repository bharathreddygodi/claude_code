/*
 * MEM-13: Implement realloc() and calloc() using malloc/free
 * realloc needs the old block size -> keep a small header in front of each block.
 * Rules to remember:
 *   realloc(NULL, n)  == malloc(n)
 *   realloc(p, 0)     -> free(p), return NULL (implementation-defined in std; common choice)
 *   on failure        -> return NULL and the ORIGINAL block stays valid
 *   calloc(n, size)   -> zeroed memory, must check n*size overflow
 * Common bug: p = realloc(p, n); leaks p on failure. Use a temp pointer.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef union { size_t size; max_align_t align; } hdr_t;   /* keeps payload aligned */

void *x_malloc(size_t n)
{
    hdr_t *h = malloc(sizeof(hdr_t) + n);
    if (!h)
        return NULL;
    h->size = n;
    return h + 1;
}

void x_free(void *p)
{
    if (p)
        free((hdr_t *)p - 1);
}

static size_t x_size(void *p) { return ((hdr_t *)p - 1)->size; }

void *x_realloc(void *p, size_t n)
{
    void *np;
    size_t old;
    if (!p)
        return x_malloc(n);
    if (n == 0) {
        x_free(p);
        return NULL;
    }
    old = x_size(p);
    if (n <= old) {                     /* shrinking: just record the new size */
        ((hdr_t *)p - 1)->size = n;
        return p;
    }
    np = x_malloc(n);
    if (!np)
        return NULL;                    /* old block untouched */
    memcpy(np, p, old);
    x_free(p);
    return np;
}

void *x_calloc(size_t count, size_t size)
{
    void *p;
    if (size && count > SIZE_MAX / size)    /* overflow check */
        return NULL;
    p = x_malloc(count * size);
    if (p)
        memset(p, 0, count * size);
    return p;
}

int main(void)
{
    int i, ok = 1;
    int *a = x_calloc(5, sizeof *a);
    for (i = 0; i < 5; i++) ok &= (a[i] == 0);
    printf("calloc zeroed        -> %s\n", ok ? "PASS" : "FAIL");

    for (i = 0; i < 5; i++) a[i] = i + 1;
    int *tmp = x_realloc(a, 10 * sizeof *a);       /* safe pattern */
    if (tmp)
        a = tmp;
    ok = 1;
    for (i = 0; i < 5; i++) ok &= (a[i] == i + 1);
    printf("realloc grow keeps data -> %s\n", ok ? "PASS" : "FAIL");

    printf("calloc overflow      -> %s\n", x_calloc(SIZE_MAX, 4) == NULL ? "NULL (PASS)" : "FAIL");
    char *s = x_realloc(NULL, 16);
    strcpy(s, "realloc(NULL)");
    printf("realloc(NULL,16)     -> %s PASS\n", s);
    printf("realloc(p,0)         -> %s\n", x_realloc(s, 0) == NULL ? "NULL, freed (PASS)" : "FAIL");
    x_free(a);
    return 0;
}
