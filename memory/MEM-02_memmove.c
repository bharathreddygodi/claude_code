/*
 * MEM-02: Implement memmove() - overlap-safe copy
 * If dst is before src (or there is no overlap) copy forward, otherwise copy backward.
 * Interview points: difference vs memcpy, why direction matters,
 * no temp buffer needed (O(1) extra space).
 */
#include <stdio.h>
#include <stddef.h>
#include <string.h>

void *my_memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s || n == 0)
        return dst;

    if (d < s) {                 /* forward copy is safe */
        while (n--)
            *d++ = *s++;
    } else {                     /* dst after src: copy from the end */
        d += n;
        s += n;
        while (n--)
            *--d = *--s;
    }
    return dst;
}

int main(void)
{
    char a[32] = "1234567890";
    char b[32] = "1234567890";

    my_memmove(a + 2, a, 5);     /* overlap, dst > src */
    memmove(b + 2, b, 5);
    printf("dst>src : %s (expected %s) -> %s\n", a, b, strcmp(a, b) ? "FAIL" : "PASS");

    strcpy(a, "1234567890");
    strcpy(b, "1234567890");
    my_memmove(a, a + 3, 5);     /* overlap, dst < src */
    memmove(b, b + 3, 5);
    printf("dst<src : %s (expected %s) -> %s\n", a, b, strcmp(a, b) ? "FAIL" : "PASS");
    return 0;
}
