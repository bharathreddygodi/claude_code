/*
 * PTR-01: Pointer basics and arithmetic
 *  - p + 1 advances by sizeof(*p) bytes, not 1 byte
 *  - p[i] is exactly *(p + i)  (so i[p] also works!)
 *  - subtracting two pointers gives an element count (ptrdiff_t), same array only
 *  - *p++ vs (*p)++ vs ++*p precedence
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int main(void)
{
    int arr[5] = {10, 20, 30, 40, 50};
    int *p = arr;
    char *c = (char *)arr;
    double d[2];
    ptrdiff_t diff;

    printf("sizeof(int*)=%u sizeof(char*)=%u sizeof(double*)=%u (all pointers same size here)\n",
           (unsigned)sizeof(int *), (unsigned)sizeof(char *), (unsigned)sizeof(double *));
    printf("int*:    p=%p p+1=%p (+%u bytes)\n", (void *)p, (void *)(p + 1), (unsigned)sizeof(int));
    printf("char*:   c=%p c+1=%p (+1 byte)\n", (void *)c, (void *)(c + 1));
    printf("double*: d=%p d+1=%p (+%u bytes)\n", (void *)d, (void *)(d + 1), (unsigned)sizeof(double));

    printf("arr[2]=%d *(arr+2)=%d 2[arr]=%d\n", arr[2], *(arr + 2), 2[arr]);

    diff = &arr[4] - &arr[1];
    printf("&arr[4] - &arr[1] = %d elements\n", (int)diff);

    p = arr;
    printf("*p++   = %d (returns 10, p moves to 20)\n", *p++);
    printf("(*p)++ = %d (returns 20, arr[1] becomes 21)\n", (*p)++);
    printf("++*p   = %d (arr[1] becomes 22)\n", ++*p);
    printf("*++p   = %d (p moves to 30)\n", *++p);
    return 0;
}
