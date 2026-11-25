/*
 * PTR-06: Arrays vs pointers (favourite trick questions)
 *  - an array "decays" to a pointer to its first element in most expressions,
 *    EXCEPT sizeof, &, and string-literal initialisation
 *  - arr and &arr have the same address but different types: int* vs int(*)[5]
 *  - sizeof(arr) inside a function taking int arr[] is sizeof(int*) !
 *  - 2D arrays: int m[3][4] decays to int (*)[4]
 *  - char s[] = "hi" (modifiable copy) vs char *s = "hi" (read-only literal)
 */
#include <stdio.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* gcc even warns about this bug (-Wsizeof-array-argument); silenced here on purpose */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-array-argument"
static void f(int arr[])          /* really: int *arr */
{
    printf("inside f: sizeof(arr) = %u (pointer!)\n", (unsigned)sizeof(arr));
}
#pragma GCC diagnostic pop

static void print_matrix(int (*m)[4], int rows)   /* same as int m[][4] */
{
    int r, c;
    for (r = 0; r < rows; r++) {
        for (c = 0; c < 4; c++) printf("%3d", m[r][c]);
        printf("\n");
    }
}

int main(void)
{
    int arr[5] = {1, 2, 3, 4, 5};
    int m[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
    char s1[] = "hi";
    const char *s2 = "hi";

    printf("sizeof(arr)=%u ARRAY_SIZE=%u\n", (unsigned)sizeof(arr), (unsigned)ARRAY_SIZE(arr));
    f(arr);
    printf("arr   =%p  arr+1   =%p (+%u)\n", (void *)arr, (void *)(arr + 1), (unsigned)sizeof(int));
    printf("&arr  =%p  &arr+1  =%p (+%u, whole array)\n", (void *)&arr, (void *)(&arr + 1), (unsigned)sizeof(arr));
    printf("array length via pointer math: (&arr)[1] - arr = %d\n", (int)((&arr)[1] - arr));

    printf("m[1][2]=%d *(*(m+1)+2)=%d sizeof(m[0])=%u\n", m[1][2], *(*(m + 1) + 2), (unsigned)sizeof(m[0]));
    print_matrix(m, 3);

    printf("sizeof(s1)=%u (array) sizeof(s2)=%u (pointer)\n", (unsigned)sizeof(s1), (unsigned)sizeof(s2));
    s1[0] = 'H';    /* fine */
    /* ((char *)s2)[0] = 'H';  UB - may crash, literal is in read-only memory */
    printf("s1=%s s2=%s\n", s1, s2);
    return 0;
}
