/*
 * BIT-12: Branchless integer tricks
 *  sign, abs, min, max without if, multiply/divide by 2^n, odd/even,
 *  add 1 without '+', add two numbers without '+'.
 * Assumes two's complement and arithmetic right shift (true for gcc/ARM/x86).
 */
#include <stdio.h>
#include <stdint.h>

int sign_of(int x)  { return (x > 0) - (x < 0); }           /* -1, 0, 1 */
int abs_bl(int x)   { int m = x >> 31; return (x ^ m) - m; } /* INT_MIN still overflows */
int min_bl(int a, int b) { return b ^ ((a ^ b) & -(a < b)); }
int max_bl(int a, int b) { return a ^ ((a ^ b) & -(a < b)); }

int add_no_plus(int a, int b)
{
    while (b) {
        unsigned carry = ((unsigned)a & (unsigned)b) << 1;
        a ^= b;
        b = (int)carry;
    }
    return a;
}

int main(void)
{
    printf("sign(-7)=%d sign(0)=%d sign(9)=%d\n", sign_of(-7), sign_of(0), sign_of(9));
    printf("abs(-42)=%d abs(42)=%d\n", abs_bl(-42), abs_bl(42));
    printf("min(3,-8)=%d max(3,-8)=%d\n", min_bl(3, -8), max_bl(3, -8));
    printf("13 is %s, 13*8=%d, 13/4=%d, 13%%4=%d\n", (13 & 1) ? "odd" : "even", 13 << 3, 13 >> 2, 13 & 3);
    printf("add_no_plus(25, 17)=%d add_no_plus(-5, 3)=%d\n", add_no_plus(25, 17), add_no_plus(-5, 3));
    printf("x+1 without '+': -~5 = %d\n", -~5);
    return 0;
}
