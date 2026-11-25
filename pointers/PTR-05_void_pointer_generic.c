/*
 * PTR-05: void* and generic programming
 *  - void* holds any object pointer; must cast before dereference; no arithmetic in std C
 *  - generic swap with size parameter
 *  - qsort with comparator, and a generic linear search
 *  - malloc returns void* -> no cast needed in C (cast hides a missing #include)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gswap(void *a, void *b, size_t size)
{
    unsigned char *p = a, *q = b, t;
    while (size--) { t = *p; *p++ = *q; *q++ = t; }
}

void *gfind(const void *base, size_t n, size_t size, const void *key,
            int (*cmp)(const void *, const void *))
{
    const unsigned char *p = base;
    size_t i;
    for (i = 0; i < n; i++, p += size)
        if (cmp(p, key) == 0) return (void *)p;
    return NULL;
}

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);      /* not x - y: that can overflow */
}

typedef struct { const char *name; int age; } person_t;
static int cmp_name(const void *a, const void *b)
{
    return strcmp(((const person_t *)a)->name, ((const person_t *)b)->name);
}

int main(void)
{
    int a = 1, b = 2, key = 7, i;
    double x = 1.5, y = 2.5;
    int arr[] = {5, -2, 9, 7, 0};
    person_t ppl[] = {{"Ravi", 30}, {"Anu", 25}, {"Kiran", 28}};

    gswap(&a, &b, sizeof a);
    gswap(&x, &y, sizeof x);
    printf("gswap int: a=%d b=%d | double: x=%.1f y=%.1f\n", a, b, x, y);

    int *f = gfind(arr, 5, sizeof arr[0], &key, cmp_int);
    printf("gfind 7 -> index %d\n", f ? (int)(f - arr) : -1);

    qsort(arr, 5, sizeof arr[0], cmp_int);
    printf("qsort ints:");
    for (i = 0; i < 5; i++) printf(" %d", arr[i]);
    qsort(ppl, 3, sizeof ppl[0], cmp_name);
    printf("\nqsort structs by name: %s %s %s\n", ppl[0].name, ppl[1].name, ppl[2].name);
    return 0;
}
