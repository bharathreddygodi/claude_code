/*
 * BIT-08: XOR classics
 *  - every number appears twice except one -> XOR all       (a^a = 0, a^0 = a)
 *  - two numbers appear once -> split by any set bit of their XOR
 *  - every number appears THREE times except one -> XOR alone fails; either
 *      a) count each bit position mod 3 (easy to explain), or
 *      b) ones/twos state machine: each bit counts 0 -> 1 -> 2 -> 0 (O(n), O(1))
 *  - missing number in 0..n -> XOR indices with values (BIT-09)
 *  - check if two ints have opposite signs -> (a ^ b) < 0
 */
#include <stdio.h>

int single_number(const int *a, int n)
{
    int x = 0, i;
    for (i = 0; i < n; i++) x ^= a[i];
    return x;
}

void two_singles(const int *a, int n, int *p, int *q)
{
    int x = 0, i;
    unsigned low;
    for (i = 0; i < n; i++) x ^= a[i];
    low = (unsigned)x & (0u - (unsigned)x);   /* lowest set bit of x */
    *p = *q = 0;
    for (i = 0; i < n; i++) {
        if ((unsigned)a[i] & low) *p ^= a[i];
        else                      *q ^= a[i];
    }
}

int single_of_three_bitcount(const int *a, int n)
{
    unsigned r = 0;
    int bit, i;
    for (bit = 0; bit < 32; bit++) {
        unsigned cnt = 0;
        for (i = 0; i < n; i++) cnt += ((unsigned)a[i] >> bit) & 1u;
        if (cnt % 3) r |= 1u << bit;       /* leftover bit belongs to the single number */
    }
    return (int)r;
}

int single_of_three_states(const int *a, int n)
{
    unsigned ones = 0, twos = 0;
    int i;
    for (i = 0; i < n; i++) {
        unsigned x = (unsigned)a[i];
        ones = (ones ^ x) & ~twos;         /* bit seen once (mod 3) */
        twos = (twos ^ x) & ~ones;         /* bit seen twice (mod 3) */
    }
    return (int)ones;
}

int missing_number(const int *a, int n)   /* a holds n distinct values from 0..n */
{
    int x = n, i;
    for (i = 0; i < n; i++) x ^= i ^ a[i];
    return x;
}

int main(void)
{
    int a[] = {4, 1, 2, 1, 2};
    int b[] = {1, 2, 1, 3, 2, 5};
    int c[] = {3, 0, 1, 5, 4};
    int d[] = {5, 9, 5, -7, 9, 5, 9};      /* -7 appears once, others three times */
    int p, q;

    printf("single_number = %d (expect 4)\n", single_number(a, 5));
    two_singles(b, 6, &p, &q);
    printf("two_singles   = %d, %d (expect 3 and 5)\n", p, q);
    printf("single of 3x  = %d (bit count) %d (ones/twos) (expect -7)\n",
           single_of_three_bitcount(d, 7), single_of_three_states(d, 7));
    printf("missing       = %d (expect 2)\n", missing_number(c, 5));
    printf("opposite signs(5,-3)=%d (5,3)=%d\n", (5 ^ -3) < 0, (5 ^ 3) < 0);
    return 0;
}
