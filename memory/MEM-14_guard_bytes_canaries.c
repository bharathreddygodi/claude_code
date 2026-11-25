/*
 * MEM-14: Guard bytes / canaries to detect buffer overruns on free
 * Each allocation is wrapped:
 *   [ header: size, magic ][ front canary ][ user data ... ][ back canary ]
 *   - back canary catches writes past the end (the common off-by-one)
 *   - front canary catches underruns (writing before index 0)
 *   - magic catches double free / freeing a pointer that wasn't allocated here
 *   - freed memory is poisoned with 0xDD so use-after-free shows up as garbage
 * Checks run in guarded_free() (and can be run periodically with heap_check()).
 * Same idea as the compiler stack protector (canary between locals and return address)
 * and FreeRTOS stack overflow check method 2 (known pattern at the end of the stack).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAGIC_LIVE  0xA110CA7Eu
#define MAGIC_FREED 0xDEADF4EEu
#define CANARY      0xC0FFEE11u

typedef struct {
    size_t size;
    uint32_t magic;
    uint32_t front;                /* front canary, directly before user data */
} ghdr_t;

void *guarded_malloc(size_t n)
{
    uint32_t back = CANARY;
    ghdr_t *h = malloc(sizeof *h + n + sizeof back);
    if (!h) return NULL;
    h->size = n;
    h->magic = MAGIC_LIVE;
    h->front = CANARY;
    memcpy((uint8_t *)(h + 1) + n, &back, sizeof back);   /* may be unaligned -> memcpy */
    return h + 1;
}

/* returns 0 ok, otherwise a bitmask: 1 underrun, 2 overrun, 4 double/invalid free */
int guarded_free(void *p, const char *tag)
{
    ghdr_t *h;
    uint32_t back;
    int err = 0;
    if (!p) return 0;
    h = (ghdr_t *)p - 1;
    if (h->magic != MAGIC_LIVE) {
        printf("  [%s] %s\n", tag, h->magic == MAGIC_FREED ? "DOUBLE FREE" : "INVALID FREE (bad magic)");
        return 4;                  /* don't touch it further */
    }
    memcpy(&back, (uint8_t *)p + h->size, sizeof back);
    if (h->front != CANARY) { printf("  [%s] UNDERRUN: front canary = 0x%08X\n", tag, (unsigned)h->front); err |= 1; }
    if (back != CANARY)     { printf("  [%s] OVERRUN : back canary  = 0x%08X\n", tag, (unsigned)back); err |= 2; }
    if (!err) printf("  [%s] ok\n", tag);
    h->magic = MAGIC_FREED;
    memset(p, 0xDD, h->size);      /* poison */
    /* real code: free(h). Kept allocated here so the double-free demo can read the
       header safely; on a real heap you'd quarantine freed blocks for the same reason. */
    return err;
}

int main(void)
{
    char *a = guarded_malloc(8);
    char *b = guarded_malloc(8);
    char *c = guarded_malloc(8);
    int *d = guarded_malloc(4 * sizeof(int));
    int i, results = 0;

    strcpy(a, "1234567");          /* 7 chars + '\0' = 8: fits */
    memcpy(b, "123456789", 10);    /* 10 bytes into 8: overrun */
    c[-1] = 'X';                   /* underrun into the front canary */
    for (i = 0; i <= 4; i++) d[i] = i;   /* classic off-by-one: <= instead of < */

    results |= guarded_free(a, "a exact fit");
    results |= guarded_free(b, "b strcpy overflow");
    results |= guarded_free(c, "c index -1");
    results |= guarded_free(d, "d off-by-one loop");
    results |= guarded_free(a, "a freed twice");
    printf("a after free (poisoned): 0x%02X 0x%02X ...\n", (unsigned char)a[0], (unsigned char)a[1]);
    printf("errors caught: underrun=%d overrun=%d double=%d\n", !!(results & 1), !!(results & 2), !!(results & 4));
    return 0;
}
