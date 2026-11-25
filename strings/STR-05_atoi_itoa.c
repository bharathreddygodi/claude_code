/*
 * STR-05: Implement atoi (with whitespace, sign and overflow handling) and itoa (any base)
 * atoi edge cases: leading spaces, '+'/'-', stop at first non-digit, clamp on overflow,
 * INT_MIN. itoa edge cases: 0, negative numbers, INT_MIN (negate as unsigned!).
 */
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

int my_atoi(const char *s)
{
    int sign = 1;
    long long v = 0;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '+' || *s == '-') sign = (*s++ == '-') ? -1 : 1;
    while (isdigit((unsigned char)*s)) {
        v = v * 10 + (*s++ - '0');
        if (sign * v > INT_MAX) return INT_MAX;
        if (sign * v < INT_MIN) return INT_MIN;
    }
    return (int)(sign * v);
}

char *my_itoa(int value, char *buf, int base)
{
    static const char digits[] = "0123456789ABCDEF";
    char *p = buf, *a, *b;
    unsigned int u;

    if (base < 2 || base > 16) { *buf = '\0'; return buf; }
    if (value < 0 && base == 10) {
        *p++ = '-';
        u = 0u - (unsigned)value;        /* safe for INT_MIN */
    } else {
        u = (unsigned)value;
    }
    a = p;
    do {
        *p++ = digits[u % (unsigned)base];
        u /= (unsigned)base;
    } while (u);
    *p = '\0';
    for (b = p - 1; a < b; a++, b--) {   /* digits came out reversed */
        char t = *a; *a = *b; *b = t;
    }
    return buf;
}

int main(void)
{
    const char *in[] = {"42", "   -123abc", "+7", "2147483648", "-2147483649", "abc", "  0012"};
    char buf[40];
    unsigned i;
    for (i = 0; i < sizeof in / sizeof in[0]; i++)
        printf("my_atoi(\"%s\") = %d\n", in[i], my_atoi(in[i]));

    printf("itoa(255,16)=%s ", my_itoa(255, buf, 16));
    printf("itoa(10,2)=%s ", my_itoa(10, buf, 2));
    printf("itoa(-450,10)=%s ", my_itoa(-450, buf, 10));
    printf("itoa(INT_MIN)=%s\n", my_itoa(INT_MIN, buf, 10));
    return 0;
}
