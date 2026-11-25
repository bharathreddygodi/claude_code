/*
 * MEM-04: Implement memset() and memcmp() efficiently
 * memset:
 *   - value is converted to unsigned char; memset(int_arr, 1, ...) does NOT put 1 in
 *     each int (you get 0x01010101); returns dst
 *   - fast version: byte-fill until aligned, then store a word-sized pattern, then tail
 * memcmp:
 *   - compare as UNSIGNED char (0x80 > 0x7F); return <0, 0, >0 at the first difference
 *   - fast version: compare words while aligned and equal, fall back to bytes to find
 *     the differing byte (needed to get the sign right on little-endian)
 *   - never memcmp structs for equality (padding bytes are indeterminate)
 */
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void *my_memset(void *dst, int c, size_t n)
{
    unsigned char *p = (unsigned char *)dst;
    while (n--)
        *p++ = (unsigned char)c;
    return dst;
}

void *my_memset_fast(void *dst, int c, size_t n)
{
    unsigned char *p = (unsigned char *)dst;
    const size_t W = sizeof(uintptr_t);
    uintptr_t pattern = (unsigned char)c;
    size_t i;

    for (i = 1; i < W; i++)                 /* 0xAB -> 0xABABABAB... */
        pattern = (pattern << 8) | (unsigned char)c;

    while (n && ((uintptr_t)p & (W - 1))) { *p++ = (unsigned char)c; n--; }
    uintptr_t *pw = (uintptr_t *)p;
    while (n >= W) { *pw++ = pattern; n -= W; }
    p = (unsigned char *)pw;
    while (n--) *p++ = (unsigned char)c;
    return dst;
}

int my_memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *p = (const unsigned char *)a;
    const unsigned char *q = (const unsigned char *)b;
    while (n--) {
        if (*p != *q)
            return (int)*p - (int)*q;
        p++;
        q++;
    }
    return 0;
}

int my_memcmp_fast(const void *a, const void *b, size_t n)
{
    const unsigned char *p = (const unsigned char *)a;
    const unsigned char *q = (const unsigned char *)b;
    const size_t W = sizeof(uintptr_t);

    if (((uintptr_t)p & (W - 1)) == 0 && ((uintptr_t)q & (W - 1)) == 0) {
        while (n >= W && *(const uintptr_t *)p == *(const uintptr_t *)q) {   /* skip equal words */
            p += W;
            q += W;
            n -= W;
        }
    }
    return my_memcmp(p, q, n);              /* bytes decide the sign */
}

static int sign(int x) { return (x > 0) - (x < 0); }

int main(void)
{
    unsigned char buf[40];
    int ok = 1, i;
    size_t k;

    /* ---- memset ---- */
    my_memset(buf, 0xAA, sizeof buf);
    for (i = 0; i < 40; i++) ok &= (buf[i] == 0xAA);
    printf("my_memset      -> %s\n", ok ? "PASS" : "FAIL");

    my_memset_fast(buf + 3, 0x5C, 30);
    ok = 1;
    for (i = 0; i < 40; i++) ok &= (buf[i] == ((i >= 3 && i < 33) ? 0x5C : 0xAA));
    printf("my_memset_fast -> %s\n", ok ? "PASS" : "FAIL");

    int arr[2];
    my_memset(arr, 1, sizeof arr);
    printf("Gotcha: memset(int[], 1) gives 0x%08X, not 1\n\n", (unsigned)arr[0]);

    /* ---- memcmp ---- */
    struct { const char *a, *b; size_t n; } t[] = {
        {"abc", "abc", 3}, {"abc", "abd", 3}, {"abd", "abc", 3},
        {"\x80", "\x7f", 1}, {"xyz", "xya", 2}, {"", "", 0},
    };
    for (k = 0; k < sizeof t / sizeof t[0]; k++) {
        int m = sign(my_memcmp(t[k].a, t[k].b, t[k].n));
        int s = sign(memcmp(t[k].a, t[k].b, t[k].n));
        printf("memcmp case %u: mine=%2d libc=%2d -> %s\n", (unsigned)k, m, s, m == s ? "PASS" : "FAIL");
    }

    {   /* fast version on aligned buffers that differ late */
        union { uintptr_t w[8]; unsigned char b[8 * sizeof(uintptr_t)]; } x, y;
        size_t len = sizeof x.b;
        memset(x.b, 7, len);
        memset(y.b, 7, len);
        y.b[len - 3] = 9;
        printf("memcmp_fast late diff: mine=%2d libc=%2d -> %s\n",
               sign(my_memcmp_fast(x.b, y.b, len)), sign(memcmp(x.b, y.b, len)),
               sign(my_memcmp_fast(x.b, y.b, len)) == sign(memcmp(x.b, y.b, len)) ? "PASS" : "FAIL");
    }
    return 0;
}
