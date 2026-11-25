/*
 * MEM-03: Implement memcpy()
 * - Copy n bytes from src to dst. Behaviour is undefined if the regions overlap.
 * - The faster version copies a word at a time once both pointers are aligned.
 * - The unrolled version copies 4 words per loop iteration: fewer branches and
 *   loop-counter updates, and lets the CPU pipeline the loads/stores (on Cortex-M
 *   this is what LDM/STM multi-register copies do). Then words, then tail bytes.
 * Interview points: no arithmetic on void* -> cast to unsigned char*,
 * return dst, handle n == 0, alignment, overlap is undefined (use memmove).
 */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Simple byte-by-byte version (what most interviewers expect first) */
void *my_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--)
        *d++ = *s++;
    return dst;
}

/* Faster version: word copy when src and dst share the same alignment */
void *my_memcpy_fast(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    const size_t W = sizeof(uintptr_t);

    if (((uintptr_t)d & (W - 1)) == ((uintptr_t)s & (W - 1))) {
        while (n && ((uintptr_t)d & (W - 1))) {   /* head bytes until aligned */
            *d++ = *s++;
            n--;
        }
        uintptr_t *dw = (uintptr_t *)d;
        const uintptr_t *sw = (const uintptr_t *)s;
        while (n >= W) {                           /* whole words */
            *dw++ = *sw++;
            n -= W;
        }
        d = (unsigned char *)dw;
        s = (const unsigned char *)sw;
    }
    while (n--)                                    /* tail bytes */
        *d++ = *s++;
    return dst;
}

/* Optimized: align head -> 4x unrolled word copy -> remaining words -> tail bytes */
void *my_memcpy_unrolled(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    const size_t W = sizeof(uintptr_t);

    if (((uintptr_t)d & (W - 1)) == ((uintptr_t)s & (W - 1))) {
        while (n && ((uintptr_t)d & (W - 1))) { *d++ = *s++; n--; }   /* head */
        uintptr_t *dw = (uintptr_t *)d;
        const uintptr_t *sw = (const uintptr_t *)s;
        while (n >= 4 * W) {                                          /* unrolled body */
            uintptr_t a = sw[0], b = sw[1], c = sw[2], e = sw[3];     /* loads first, then stores */
            dw[0] = a; dw[1] = b; dw[2] = c; dw[3] = e;
            dw += 4; sw += 4; n -= 4 * W;
        }
        while (n >= W) { *dw++ = *sw++; n -= W; }                     /* leftover words */
        d = (unsigned char *)dw;
        s = (const unsigned char *)sw;
    }
    while (n--) *d++ = *s++;                                          /* tail */
    return dst;
}

int main(void)
{
    char src[] = "Hello, embedded world! 0123456789";
    char d1[64] = {0}, d2[64] = {0};

    my_memcpy(d1, src, sizeof src);
    my_memcpy_fast(d2, src, sizeof src);

    printf("my_memcpy      : %s -> %s\n", d1, strcmp(d1, src) == 0 ? "PASS" : "FAIL");
    printf("my_memcpy_fast : %s -> %s\n", d2, strcmp(d2, src) == 0 ? "PASS" : "FAIL");

    {   /* every length 0..100 and every start offset 0..7, compared with libc */
        unsigned char a[128], b[128], c[128];
        size_t len, off, k;
        int ok = 1;
        for (k = 0; k < sizeof a; k++) a[k] = (unsigned char)(k * 7 + 3);
        for (off = 0; off < 8; off++)
            for (len = 0; len <= 100; len++) {
                memset(b, 0, sizeof b);
                memset(c, 0, sizeof c);
                my_memcpy_unrolled(b + off, a + off, len);
                memcpy(c + off, a + off, len);
                ok &= memcmp(b, c, sizeof b) == 0;
            }
        printf("my_memcpy_unrolled, 808 length/offset combos -> %s\n", ok ? "PASS" : "FAIL");
    }
    return 0;
}
