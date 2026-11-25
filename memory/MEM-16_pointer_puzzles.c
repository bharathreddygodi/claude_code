/*
 * MEM-16: Pointer puzzles
 * 1) int (*p)[10]  -> ONE pointer to an array of 10 ints   (p + 1 moves 40 bytes)
 *    int *p[10]    -> an ARRAY of 10 pointers to int        (sizeof = 10 pointers)
 * 2) void * arithmetic: not allowed in standard C (void has no size). GCC allows it as
 *    an extension and treats it as char* (+1 byte) - say this, then cast to (char *) or (uint8_t *).
 *    (Gotcha: writing "char*" + "/" inside a comment closes the comment early!)
 * 3) char * aliasing / strict aliasing:
 *    - you may inspect ANY object through char / unsigned char *  (always legal)
 *    - reading an object through an unrelated pointer type (float via uint32_t*) is UB;
 *      -O2 may reorder or cache values. Use memcpy (or a union in C) to type-pun.
 *    - pointers of the same type may alias, so the compiler must reload after writes
 *      through them; `restrict` promises they don't.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* aliasing demo: compiler must assume a and b may point to the same int */
static int may_alias(int *a, int *b)
{
    *a = 1;
    *b = 2;
    return *a;          /* must reload: returns 2 if a == b */
}

static uint32_t float_bits_memcpy(float f)
{
    uint32_t u;
    memcpy(&u, &f, sizeof u);   /* well-defined type punning; compiles to one move */
    return u;
}

static uint32_t float_bits_union(float f)
{
    union { float f; uint32_t u; } x;
    x.f = f;
    return x.u;                 /* allowed in C (not in C++) */
}

int main(void)
{
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int m[3][10] = {{0}};
    int (*pa)[10] = &arr;       /* pointer to array */
    int *ap[10];                /* array of pointers */
    int (*rows)[10] = m;        /* how a 2D array decays */
    int i, x = 0;
    uint32_t word = 0x11223344u;
    const unsigned char *bytes = (const unsigned char *)&word;
    void *vp = arr;

    for (i = 0; i < 10; i++) ap[i] = &arr[9 - i];

    printf("--- int (*p)[10] vs int *p[10] ---\n");
    printf("sizeof(pa)=%u (one pointer)  sizeof(ap)=%u (10 pointers)\n", (unsigned)sizeof pa, (unsigned)sizeof ap);
    printf("sizeof(*pa)=%u (whole array)  (*pa)[3]=%d  *ap[3]=%d\n", (unsigned)sizeof *pa, (*pa)[3], *ap[3]);
    printf("pa+1 moves %d bytes; ap+1 moves %d bytes\n",
           (int)((char *)(pa + 1) - (char *)pa), (int)((char *)(ap + 1) - (char *)ap));
    rows[2][5] = 42;
    printf("rows+2 is row 2: rows[2][5]=%d (m[2][5]=%d)\n", rows[2][5], m[2][5]);

    printf("--- void * arithmetic ---\n");
    printf("standard way: (char *)vp + 4 -> element %d\n", *(int *)((char *)vp + 4));
    /* vp + 4 would compile on GCC only (extension, -Wpointer-arith warns); not portable */

    printf("--- char * aliasing ---\n");
    printf("0x11223344 bytes via unsigned char*: %02X %02X %02X %02X (always legal)\n",
           bytes[0], bytes[1], bytes[2], bytes[3]);
    printf("1.0f bits via memcpy = 0x%08X, via union = 0x%08X\n",
           (unsigned)float_bits_memcpy(1.0f), (unsigned)float_bits_union(1.0f));
    printf("may_alias(&x, &x) = %d (compiler reloads because same-type pointers may alias)\n",
           may_alias(&x, &x));
    /* UB example (don't do it): uint32_t u = *(uint32_t *)&some_float; */
    return 0;
}
