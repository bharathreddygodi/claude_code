/*
 * MEM-01: Implement aligned_malloc(size, align) and aligned_free(ptr)
 * Trick: over-allocate (size + align - 1 + sizeof(void*)), round the address up,
 * and store the ORIGINAL pointer just before the aligned block so free can find it.
 * Interview points: align must be a power of 2, rounding with (x + a-1) & ~(a-1),
 * why the original pointer is stashed, used for DMA / cache-line aligned buffers.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void *aligned_malloc(size_t size, size_t align)
{
    if (align == 0 || (align & (align - 1)))     /* must be a power of two */
        return NULL;

    void *raw = malloc(size + align - 1 + sizeof(void *));
    if (!raw)
        return NULL;

    uintptr_t start = (uintptr_t)raw + sizeof(void *);
    uintptr_t aligned = (start + align - 1) & ~(uintptr_t)(align - 1);

    ((void **)aligned)[-1] = raw;                /* stash the original pointer */
    return (void *)aligned;
}

void aligned_free(void *p)
{
    if (p)
        free(((void **)p)[-1]);
}

int main(void)
{
    size_t aligns[] = {1, 2, 8, 16, 32, 64, 256, 4096};
    size_t i;
    for (i = 0; i < sizeof aligns / sizeof aligns[0]; i++) {
        void *p = aligned_malloc(100, aligns[i]);
        int ok = p && ((uintptr_t)p % aligns[i] == 0);
        printf("align %4u -> %p %s\n", (unsigned)aligns[i], p, ok ? "PASS" : "FAIL");
        aligned_free(p);
    }
    printf("align 3 (invalid) -> %s\n", aligned_malloc(10, 3) == NULL ? "PASS (NULL)" : "FAIL");
    return 0;
}
